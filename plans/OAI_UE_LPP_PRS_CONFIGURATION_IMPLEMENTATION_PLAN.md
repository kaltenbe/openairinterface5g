# UE PRS Configuration from LPP Assistance Data

## Objective

Configure the NR UE PRS receiver from decoded
`NR-DL-PRS-AssistanceData-r16`, while preserving the existing configuration-file
workflow and keeping the implementation independent of the LPP delivery
transport.

The first delivery path is PosSIB. A future SUPL implementation must be able to
provide the same decoded assistance data without adding another PHY-specific
conversion path.

## Current State

The UE currently receives its PRS configuration from `RCconfig_nrUE_prs()` in
`openair1/PHY/INIT/nr_init_ue.c`. That function performs three jobs at once:

1. It parses the configuration file.
2. It writes directly into PHY-owned `NR_UE_PRS` and `prs_config_t` objects.
3. It logs the resulting configuration.

The PRS scheduler in `openair1/SCHED_NR_UE/phy_procedures_nr_ue.c` reads those
objects directly on every slot. Channel estimation in
`openair1/PHY/NR_UE_ESTIMATION/nr_dl_channel_estimation.c` also reads the same
objects.

The UE can decode the PosSIB and its inner LPP PRS assistance data, but there is
currently no transport-independent path that validates, stores, and applies the
decoded information to the UE PHY.

The gNB-side `openair2/GNB_APP/lpp_prs.c` already contains the forward mapping
from `prs_config_t` into LPP ASN.1. Much of the new conversion logic can be
implemented as the tested inverse of those mappings.

## Proposed Architecture

```text
Configuration file ------------------+
                                      |
PosSIB -> LPP decode -----------------+-> normalized PRS model
                                      |          |
SUPL -> LPP decode (future) ----------+          v
                                           validate/select
                                                  |
                                                  v
                                             PHY PRS plan
                                                  |
                                                  v
                                      atomic slot-boundary apply
```

There should be four distinct responsibilities:

1. **Transport adapters** extract `NR-DL-PRS-AssistanceData-r16` from PosSIB,
   SUPL, or another delivery mechanism.
2. **LPP conversion** copies the ASN.1 data into an owned, ASN.1-free normalized
   model.
3. **Validation and compilation** turn the normalized data into the existing
   `prs_config_t`-based PHY representation.
4. **Runtime application** replaces the active PHY plan atomically at a slot
   boundary.

Neither PosSIB nor SUPL code should write into `PHY_VARS_NR_UE` directly.

## 1. Transport-Independent Normalized Model

Add an ASN.1-free model in a common header, for example:

```text
openair2/COMMON/nr_ue_prs_config.h
```

An indicative definition is:

```c
typedef enum {
  NR_PRS_SOURCE_CONFIG_FILE,
  NR_PRS_SOURCE_POS_SIB,
  NR_PRS_SOURCE_SUPL,
} nr_prs_config_source_t;

typedef struct {
  uint8_t resource_id;
  prs_config_t phy;
} nr_ue_prs_resource_config_t;

typedef struct {
  uint16_t dl_prs_id;
  uint8_t resource_set_id;

  /* Retained even when the current PHY does not consume them. */
  uint32_t point_a_arfcn;
  uint32_t trp_arfcn;
  uint16_t phys_cell_id;
  uint8_t subcarrier_spacing;
  bool normal_cyclic_prefix;

  uint8_t num_resources;
  nr_ue_prs_resource_config_t resources[NR_MAX_PRS_RESOURCES_PER_SET];
} nr_ue_prs_target_config_t;

typedef struct {
  nr_prs_config_source_t source;
  uint32_t generation;
  uint8_t num_targets;
  nr_ue_prs_target_config_t targets[NR_MAX_PRS_TARGETS];
} nr_ue_prs_configuration_t;
```

The existing `prs_config_t` remains the executable PHY representation. The
surrounding model retains LPP identities, frequency information, and other
metadata needed for future multi-TRP, multi-resource-set, SUPL, and assistance
data update support.

Add explicit muting-pattern lengths to the normalized or compiled structure.
The existing fixed-size arrays contain the pattern bits but do not retain their
actual lengths.

The array `PHY_VARS_NR_UE::prs_vars` currently uses
`NR_MAX_PRS_COMB_SIZE` as its number of PRS transmitters. Introduce an explicit
`NR_MAX_PRS_TARGETS` limit instead of coupling the target count to the maximum
comb size.

## 2. Reusable LPP Conversion

Add a reusable converter, likely under `openair3/LPP`:

```c
bool lpp_nr_prs_assistance_to_config(
    const LPP_NR_DL_PRS_AssistanceData_r16_t *assistance,
    nr_prs_config_source_t source,
    nr_ue_prs_configuration_t *out,
    nr_prs_config_error_t *error);
```

The converter must:

- validate ASN.1 choices and optional fields before dereferencing them;
- copy all required data into the normalized model;
- retain no pointers into the ASN.1 object;
- return a structured error identifying the unsupported or invalid field;
- leave `out` unusable or cleared on failure;
- never partially apply a configuration.

The decoded ASN.1 tree can therefore be freed immediately after conversion.

The PosSIB path becomes conceptually:

```c
decode_possib(..., &assistance);
lpp_nr_prs_assistance_to_config(
    assistance, NR_PRS_SOURCE_POS_SIB, &config, &error);
nr_ue_submit_prs_configuration(ue_id, &config);
```

A future SUPL path uses the same functions:

```c
decode_lpp_message(..., &assistance);
lpp_nr_prs_assistance_to_config(
    assistance, NR_PRS_SOURCE_SUPL, &config, &error);
nr_ue_submit_prs_configuration(ue_id, &config);
```

Only extraction of the inner assistance object is transport-specific.

## 3. LPP-to-PHY Field Mapping

Implement inverse mapping helpers corresponding to the gNB-side helpers in
`openair2/GNB_APP/lpp_prs.c`.

| LPP field | Internal field or operation |
| --- | --- |
| `dl-PRS-ResourceBandwidth-r16` | `NumRB = 4 * encoded + 20` |
| `dl-PRS-StartPRB-r16` | `RBOffset` |
| Frequency-layer comb size | `CombSize` |
| Periodicity choice and value | `PRSResourceSetPeriod[0]` |
| Resource-set slot offset | `PRSResourceSetPeriod[1]` |
| Repetition factor absent | `PRSResourceRepetition = 1` |
| Repetition factor present | Mapped value 2, 4, 6, 8, 16, or 32 |
| Resource time gap absent | `PRSResourceTimeGap = 1` |
| Resource time gap present | Mapped value 1, 2, 4, 8, 16, or 32 |
| Number of symbols | `NumPRSSymbols` |
| Resource slot offset | `PRSResourceOffset` |
| Resource symbol offset | `SymbolStart` |
| Sequence ID | `NPRSID` |
| Comb-specific RE offset | `REOffset` |
| Muting option 1 | Pattern 1, length, and repetition factor |
| Muting option 2 | Pattern 2 and length |

Also retain, without requiring immediate PHY support:

- DL-PRS ID;
- resource-set ID and resource ID;
- frequency-layer identity;
- TRP PCI, NCGI, and ARFCN;
- Point A and subcarrier spacing;
- cyclic prefix;
- QCL information;
- expected RSTD and uncertainty;
- PRS resource power;
- SSB association and SSB configuration.

These values will be needed for selecting measurement targets, reporting
measurements, and supporting assistance data that is more general than the
initial generated PosSIB.

## 4. Refactor the Configuration-File Path

Split `RCconfig_nrUE_prs()` into parsing, validation/compilation, application,
and logging functions:

```c
bool nr_ue_prs_config_from_file(nr_ue_prs_configuration_t *out);

bool nr_ue_prs_validate_and_compile(
    const nr_ue_prs_configuration_t *input,
    nr_ue_prs_phy_plan_t *plan,
    nr_prs_config_error_t *error);

void nr_ue_prs_log_plan(const nr_ue_prs_phy_plan_t *plan);
```

At startup the flow is:

```c
nr_ue_prs_config_from_file(&config);
nr_ue_prs_validate_and_compile(&config, &plan, &error);
nr_ue_prs_apply_initial(ue, &plan);
```

At runtime, LPP-derived input calls the same validator/compiler and application
logic. This preserves the configuration-file format, `prs_config_t`, scheduler,
channel-estimation code, and measurement storage.

The parser should no longer write individual fields directly into the active
PHY objects.

## 5. RRC-to-MAC-to-PHY Delivery

Follow the existing SIB19 configuration pattern, but use a dedicated PRS
message:

1. Add `NR_MAC_RRC_CONFIG_PRS` to the RRC-to-MAC message types.
2. Put an owned `nr_ue_prs_configuration_t` in the message.
3. Let MAC store the accepted source, generation, and normalized configuration.
4. Validate and compile before changing the active configuration.
5. Invoke a dedicated MAC-to-PHY `prs_config_request` callback.

A dedicated callback is preferable to embedding the complete PRS assistance
model in `fapi_nr_config_request_t`: PRS assistance is not normal serving-cell
configuration and will eventually contain several frequency layers, TRPs, and
resource sets.

Expose a transport-neutral submission API rather than requiring future SUPL
code to call an RRC-private static function. The implementation should enqueue
the request onto the existing serialized UE control path so callers from other
tasks do not update MAC or PHY synchronously.

## 6. Atomic Runtime Replacement

The current PHY scheduler reads PRS configuration on every slot. Updating the
same memory from a control-plane thread can expose a mixture of old and new
values.

Use an active/pending plan owned by PHY:

```c
typedef struct {
  nr_ue_prs_phy_plan_t active;
  nr_ue_prs_phy_plan_t pending;
  atomic_bool pending_valid;
} nr_ue_prs_runtime_t;
```

The MAC-to-PHY callback copies an already validated plan into `pending`. The UE
processing thread swaps or copies it into `active` at the beginning of a slot.
An invalid configuration leaves the active plan unchanged.

Measurement allocations can remain stable. During the slot-boundary update,
reset measurements for changed or removed resources and then publish the new
resource counts and configuration.

The scheduler and channel-estimation call chain must use one consistent active
plan snapshot for a slot. If required, pass the selected `prs_config_t` and
measurement object into channel estimation rather than looking the
configuration up again through `PHY_VARS_NR_UE`.

## 7. Initial Supported Subset

The first implementation should explicitly support the subset representable by
the current receiver:

- the UE's current serving frequency and numerology;
- normal cyclic prefix;
- supported PRS bandwidth, comb size, symbol count, repetition, gap, and
  periodicity values;
- resource bandwidth and offset fitting the UE carrier;
- at most `NR_MAX_PRS_TARGETS` targets;
- at most `NR_MAX_PRS_RESOURCES_PER_SET` resources per set;
- one or more TRPs flattened into the current PHY target dimension;
- initially one resource set per TRP, if minimizing the first change is
  desirable.

Reject unsupported multi-frequency input, unsupported multiple sets, duplicate
identities, invalid offsets, and capacity excess as a complete transaction.
Never silently truncate assistance data or retain a partially converted plan.

The normalized model should nevertheless retain fields not yet consumed by
PHY so that later extensions do not require redesigning the transport and
conversion interfaces.

## 8. Configuration Source and Lifetime Policy

Make arbitration explicit:

- The configuration file is the bootstrap and fallback source.
- PosSIB replaces broadcast-derived configuration for the associated serving
  cell.
- SUPL may later have higher priority for the lifetime of its positioning
  transaction.
- Repeated identical PosSIB deliveries are ignored using a generation, value
  tag, or stable configuration hash.
- PosSIB invalidation, a serving-cell change, or SUPL transaction completion
  restores the next applicable source.
- Reset and handover remove configurations tied to the old serving cell.

Source selection belongs above PHY. PHY should receive only the currently
selected compiled plan.

## 9. Existing Functional Gaps

The current scheduler uses periodicity, resource offsets, repetitions, and time
gap. It does not consult `MutingPattern1`, `MutingPattern2`, or
`MutingBitRepetition`; those fields are currently only parsed and logged.

Receiving muting information through LPP will therefore not make it effective
until PRS slot eligibility implements the Rel-16 muting calculation.

Keep these as two separately testable changes:

1. Assistance-data conversion and atomic configuration replacement.
2. Muting enforcement and additional resource-selection behavior.

## 10. Validation

Validation should be shared by config-file and LPP input wherever possible and
cover at least:

- nonzero target, set, and resource counts;
- all array and implementation limits;
- unique target, set, and resource identities;
- supported subcarrier spacing and cyclic prefix;
- periodicity choice matching the frequency-layer subcarrier spacing;
- resource-set offset smaller than the period;
- valid repetition/time-gap combinations;
- `SymbolStart + NumPRSSymbols <= 14`;
- `REOffset < CombSize`;
- sequence ID in `[0, 4095]`;
- resource slot offset in its ASN.1 and implementation ranges;
- PRS bandwidth in 24 to 272 PRBs in steps of four;
- start PRB and bandwidth fitting the current carrier;
- valid muting pattern lengths and binary bit values;
- frequency-layer comb size agreeing with every resource;
- resource-set common fields consistently applied to every resource.

Errors should identify the frequency layer, TRP, resource set, resource, and
field when available.

## 11. Tests

### Baseline RFsim test

Before changing the PRS configuration path, preserve the existing
configuration-file behavior with the repository-level `test_rfsim.sh` test.
The test uses:

- `gnb.sa.band254.u0.25prb.rfsim.ntn-leo-RegenWithPRS.conf` for the gNB;
- `ue_Leo_Regen.conf` for the UE;
- the existing UE configuration hierarchy
  `PRSs -> Active_gNBs -> prs_config0`;
- `DL PRS ToA` in the UE log as the required observable result.

`test_rfsim.sh` must fail if the UE configuration file is missing, either OAI
process exits unexpectedly, or the UE log does not contain `DL PRS ToA`. This
test is the baseline proving that PRS transmission, reception, scheduling, and
ToA estimation work before assistance data is applied dynamically.

Run it before the implementation begins and after every refactoring step that
touches the config-file loader, PHY PRS storage, scheduling, or channel
estimation:

```bash
./test_rfsim.sh
```

For a quick local regression run, the duration can be shortened while retaining
the same assertion:

```bash
TEST_DURATION=5 ./test_rfsim.sh
```

### PosSIB-only RFsim regression

The companion PosSIB mode uses `ue_Leo_Regen_possib.conf`, which deliberately
omits the `PRSs` section. It requires both successful PosSIB configuration and
the existing ToA observable:

```bash
PRS_INPUT=possib TEST_DURATION=5 ./test_rfsim.sh
```

This mode must remain separate from the default `PRS_INPUT=config` baseline.
It proves that the UE did not obtain PRS from its file and that the decoded
`NR-DL-PRS-AssistanceData-r16` was applied before `DL PRS ToA` is produced.

### Conversion unit tests

- Decode or construct representative LPP assistance data and verify every
  normalized and PHY field.
- Test all supported comb sizes, symbol counts, repetitions, gaps,
  periodicities, and muting lengths.
- Verify ASN.1 defaults, especially absent repetition and time-gap fields.
- Test malformed choices, missing required pointers, invalid ranges, duplicate
  IDs, and capacity excess.
- Confirm that conversion output remains valid after the ASN.1 tree is freed.

### Mapping symmetry tests

Use configurations accepted by the gNB-side generator:

```text
prs_config_t -> LPP ASN.1 -> normalized model -> PHY plan
```

The resulting executable PRS fields should equal the original configuration.

### Configuration-file regression tests

- Parse existing UE PRS config files through the new path.
- Compare the compiled plan against the structures populated by the current
  implementation.
- Verify that startup behavior remains unchanged when no runtime assistance is
  received.
- Run `test_rfsim.sh` and require at least one `DL PRS ToA` UE log entry.

### Runtime tests

- Apply a plan while the UE is running and verify that no slot observes a
  partially updated configuration.
- Verify idempotence for repeated PosSIB delivery.
- Verify fallback after invalidation or cell change.
- Verify that invalid assistance leaves the last valid plan active.
- Verify measurement reset for removed or changed resources.

### End-to-end test

- Generate PosSIB from a known gNB PRS configuration.
- Deliver and decode it at the UE.
- Verify the applied UE plan against the gNB configuration.
- Receive PRS and confirm that the expected target/resource measurement is
  produced.
- Extend `test_rfsim.sh`, or add a companion mode, that removes the UE
  config-file PRS section and requires `DL PRS ToA` after the same configuration
  has been received through PosSIB. Keep the config-file mode as a separate
  baseline so a dynamic-configuration failure can be distinguished from a PHY
  PRS regression.

## 12. Implementation Sequence

1. Run and preserve the config-file `test_rfsim.sh` baseline, including its
   `DL PRS ToA` assertion.
2. Add the normalized model, explicit limits, errors, and common validator.
3. Refactor the configuration-file loader to produce the normalized model and
   use the common compiler, without changing behavior.
4. Add inverse LPP mapping helpers and conversion unit tests.
5. Add the RRC-to-MAC PRS message and transport-neutral submission API.
6. Add the dedicated MAC-to-PHY request and pending/active PHY plan.
7. Connect the existing PosSIB decoder to the common LPP converter.
8. Add a PosSIB RFsim mode without config-file PRS input and require
   `DL PRS ToA`.
9. Add source arbitration, duplicate detection, invalidation, and fallback.
10. Implement PRS muting behavior.
11. Extend the compiler and PHY representation for multiple resource sets and
   frequency layers when receiver support is added.

## Expected Result

After the initial implementation, the existing UE configuration files continue
to work, while successfully decoded PosSIB assistance data can replace the
active PRS receiver configuration at runtime. The PHY remains independent of
ASN.1 and of the delivery transport. A future SUPL client only needs to extract
the same LPP assistance object and submit it through the shared conversion and
configuration path.
