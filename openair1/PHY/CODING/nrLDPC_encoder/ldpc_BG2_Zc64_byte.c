#include "PHY/sse_intrin.h"
// generated code for Zc=64, byte encoding
static inline void ldpc_BG2_Zc64_byte(uint8_t *c,uint8_t *d) {
  __m256i *csimd=(__m256i *)c,*dsimd=(__m256i *)d;

  __m256i *c2,*d2;

  int i2;
  for (i2=0; i2<2; i2++) {
     c2=&csimd[i2];
     d2=&dsimd[i2];

//row: 0
     d2[0]=_mm256_xor_si256(c2[256],_mm256_xor_si256(c2[193],_mm256_xor_si256(c2[512],_mm256_xor_si256(c2[645],_mm256_xor_si256(c2[549],_mm256_xor_si256(c2[228],_mm256_xor_si256(c2[360],_mm256_xor_si256(c2[809],_mm256_xor_si256(c2[812],_mm256_xor_si256(c2[173],_mm256_xor_si256(c2[365],_mm256_xor_si256(c2[913],_mm256_xor_si256(c2[625],_mm256_xor_si256(c2[944],_mm256_xor_si256(c2[917],_mm256_xor_si256(c2[245],_mm256_xor_si256(c2[921],_mm256_xor_si256(c2[57],_mm256_xor_si256(c2[536],_mm256_xor_si256(c2[892],_mm256_xor_si256(c2[701],_mm256_xor_si256(c2[1024],_mm256_xor_si256(c2[513],_mm256_xor_si256(c2[576],_mm256_xor_si256(c2[420],_mm256_xor_si256(c2[901],c2[1029]))))))))))))))))))))))))));

//row: 1
     d2[2]=_mm256_xor_si256(c2[288],_mm256_xor_si256(c2[256],_mm256_xor_si256(c2[193],_mm256_xor_si256(c2[512],_mm256_xor_si256(c2[677],_mm256_xor_si256(c2[645],_mm256_xor_si256(c2[549],_mm256_xor_si256(c2[228],_mm256_xor_si256(c2[392],_mm256_xor_si256(c2[360],_mm256_xor_si256(c2[809],_mm256_xor_si256(c2[844],_mm256_xor_si256(c2[812],_mm256_xor_si256(c2[173],_mm256_xor_si256(c2[365],_mm256_xor_si256(c2[913],_mm256_xor_si256(c2[625],_mm256_xor_si256(c2[944],_mm256_xor_si256(c2[917],_mm256_xor_si256(c2[245],_mm256_xor_si256(c2[953],_mm256_xor_si256(c2[921],_mm256_xor_si256(c2[57],_mm256_xor_si256(c2[536],_mm256_xor_si256(c2[892],_mm256_xor_si256(c2[701],_mm256_xor_si256(c2[1024],_mm256_xor_si256(c2[513],_mm256_xor_si256(c2[576],_mm256_xor_si256(c2[452],_mm256_xor_si256(c2[420],_mm256_xor_si256(c2[901],c2[1029]))))))))))))))))))))))))))))))));

//row: 2
     d2[4]=_mm256_xor_si256(c2[288],_mm256_xor_si256(c2[256],_mm256_xor_si256(c2[225],_mm256_xor_si256(c2[193],_mm256_xor_si256(c2[512],_mm256_xor_si256(c2[677],_mm256_xor_si256(c2[645],_mm256_xor_si256(c2[549],_mm256_xor_si256(c2[228],_mm256_xor_si256(c2[392],_mm256_xor_si256(c2[360],_mm256_xor_si256(c2[809],_mm256_xor_si256(c2[844],_mm256_xor_si256(c2[812],_mm256_xor_si256(c2[205],_mm256_xor_si256(c2[173],_mm256_xor_si256(c2[365],_mm256_xor_si256(c2[945],_mm256_xor_si256(c2[913],_mm256_xor_si256(c2[625],_mm256_xor_si256(c2[944],_mm256_xor_si256(c2[949],_mm256_xor_si256(c2[917],_mm256_xor_si256(c2[245],_mm256_xor_si256(c2[953],_mm256_xor_si256(c2[921],_mm256_xor_si256(c2[89],_mm256_xor_si256(c2[57],_mm256_xor_si256(c2[536],_mm256_xor_si256(c2[924],_mm256_xor_si256(c2[892],_mm256_xor_si256(c2[701],_mm256_xor_si256(c2[33],_mm256_xor_si256(c2[1024],_mm256_xor_si256(c2[513],_mm256_xor_si256(c2[576],_mm256_xor_si256(c2[452],_mm256_xor_si256(c2[420],_mm256_xor_si256(c2[933],_mm256_xor_si256(c2[901],c2[1029]))))))))))))))))))))))))))))))))))))))));

//row: 3
     d2[6]=_mm256_xor_si256(c2[256],_mm256_xor_si256(c2[193],_mm256_xor_si256(c2[512],_mm256_xor_si256(c2[645],_mm256_xor_si256(c2[549],_mm256_xor_si256(c2[260],_mm256_xor_si256(c2[228],_mm256_xor_si256(c2[360],_mm256_xor_si256(c2[841],_mm256_xor_si256(c2[809],_mm256_xor_si256(c2[812],_mm256_xor_si256(c2[173],_mm256_xor_si256(c2[365],_mm256_xor_si256(c2[913],_mm256_xor_si256(c2[625],_mm256_xor_si256(c2[976],_mm256_xor_si256(c2[944],_mm256_xor_si256(c2[917],_mm256_xor_si256(c2[277],_mm256_xor_si256(c2[245],_mm256_xor_si256(c2[921],_mm256_xor_si256(c2[57],_mm256_xor_si256(c2[568],_mm256_xor_si256(c2[536],_mm256_xor_si256(c2[892],_mm256_xor_si256(c2[733],_mm256_xor_si256(c2[701],_mm256_xor_si256(c2[1024],_mm256_xor_si256(c2[513],_mm256_xor_si256(c2[608],_mm256_xor_si256(c2[576],_mm256_xor_si256(c2[420],_mm256_xor_si256(c2[901],_mm256_xor_si256(c2[36],c2[1029]))))))))))))))))))))))))))))))))));

//row: 4
     d2[8]=_mm256_xor_si256(c2[512],_mm256_xor_si256(c2[480],_mm256_xor_si256(c2[417],_mm256_xor_si256(c2[736],_mm256_xor_si256(c2[609],_mm256_xor_si256(c2[901],_mm256_xor_si256(c2[869],_mm256_xor_si256(c2[773],_mm256_xor_si256(c2[452],_mm256_xor_si256(c2[708],_mm256_xor_si256(c2[616],_mm256_xor_si256(c2[584],_mm256_xor_si256(c2[8],_mm256_xor_si256(c2[45],_mm256_xor_si256(c2[13],_mm256_xor_si256(c2[397],_mm256_xor_si256(c2[589],_mm256_xor_si256(c2[112],_mm256_xor_si256(c2[849],_mm256_xor_si256(c2[145],_mm256_xor_si256(c2[116],_mm256_xor_si256(c2[469],_mm256_xor_si256(c2[152],_mm256_xor_si256(c2[120],_mm256_xor_si256(c2[281],_mm256_xor_si256(c2[760],_mm256_xor_si256(c2[93],_mm256_xor_si256(c2[925],_mm256_xor_si256(c2[225],_mm256_xor_si256(c2[737],_mm256_xor_si256(c2[800],_mm256_xor_si256(c2[676],_mm256_xor_si256(c2[644],_mm256_xor_si256(c2[100],c2[228]))))))))))))))))))))))))))))))))));

//row: 5
     d2[10]=_mm256_xor_si256(c2[513],_mm256_xor_si256(c2[481],_mm256_xor_si256(c2[416],_mm256_xor_si256(c2[737],_mm256_xor_si256(c2[225],_mm256_xor_si256(c2[900],_mm256_xor_si256(c2[868],_mm256_xor_si256(c2[772],_mm256_xor_si256(c2[453],_mm256_xor_si256(c2[293],_mm256_xor_si256(c2[617],_mm256_xor_si256(c2[585],_mm256_xor_si256(c2[9],_mm256_xor_si256(c2[44],_mm256_xor_si256(c2[12],_mm256_xor_si256(c2[396],_mm256_xor_si256(c2[588],_mm256_xor_si256(c2[113],_mm256_xor_si256(c2[848],_mm256_xor_si256(c2[144],_mm256_xor_si256(c2[117],_mm256_xor_si256(c2[468],_mm256_xor_si256(c2[84],_mm256_xor_si256(c2[153],_mm256_xor_si256(c2[121],_mm256_xor_si256(c2[280],_mm256_xor_si256(c2[761],_mm256_xor_si256(c2[92],_mm256_xor_si256(c2[924],_mm256_xor_si256(c2[1020],_mm256_xor_si256(c2[224],_mm256_xor_si256(c2[736],_mm256_xor_si256(c2[801],_mm256_xor_si256(c2[677],_mm256_xor_si256(c2[645],_mm256_xor_si256(c2[101],c2[229]))))))))))))))))))))))))))))))))))));

//row: 6
     d2[12]=_mm256_xor_si256(c2[225],_mm256_xor_si256(c2[193],_mm256_xor_si256(c2[128],_mm256_xor_si256(c2[449],_mm256_xor_si256(c2[864],_mm256_xor_si256(c2[612],_mm256_xor_si256(c2[580],_mm256_xor_si256(c2[484],_mm256_xor_si256(c2[165],_mm256_xor_si256(c2[329],_mm256_xor_si256(c2[297],_mm256_xor_si256(c2[744],_mm256_xor_si256(c2[781],_mm256_xor_si256(c2[749],_mm256_xor_si256(c2[108],_mm256_xor_si256(c2[300],_mm256_xor_si256(c2[848],_mm256_xor_si256(c2[560],_mm256_xor_si256(c2[881],_mm256_xor_si256(c2[852],_mm256_xor_si256(c2[180],_mm256_xor_si256(c2[149],_mm256_xor_si256(c2[888],_mm256_xor_si256(c2[856],_mm256_xor_si256(c2[1017],_mm256_xor_si256(c2[473],_mm256_xor_si256(c2[829],_mm256_xor_si256(c2[636],_mm256_xor_si256(c2[445],_mm256_xor_si256(c2[961],_mm256_xor_si256(c2[448],_mm256_xor_si256(c2[513],_mm256_xor_si256(c2[389],_mm256_xor_si256(c2[357],_mm256_xor_si256(c2[836],_mm256_xor_si256(c2[964],c2[932]))))))))))))))))))))))))))))))))))));

//row: 7
     d2[14]=_mm256_xor_si256(c2[384],_mm256_xor_si256(c2[352],_mm256_xor_si256(c2[897],_mm256_xor_si256(c2[289],_mm256_xor_si256(c2[832],_mm256_xor_si256(c2[608],_mm256_xor_si256(c2[128],_mm256_xor_si256(c2[773],_mm256_xor_si256(c2[741],_mm256_xor_si256(c2[261],_mm256_xor_si256(c2[645],_mm256_xor_si256(c2[165],_mm256_xor_si256(c2[901],_mm256_xor_si256(c2[324],_mm256_xor_si256(c2[869],_mm256_xor_si256(c2[36],_mm256_xor_si256(c2[488],_mm256_xor_si256(c2[456],_mm256_xor_si256(c2[1001],_mm256_xor_si256(c2[457],_mm256_xor_si256(c2[905],_mm256_xor_si256(c2[425],_mm256_xor_si256(c2[940],_mm256_xor_si256(c2[908],_mm256_xor_si256(c2[428],_mm256_xor_si256(c2[269],_mm256_xor_si256(c2[812],_mm256_xor_si256(c2[461],_mm256_xor_si256(c2[1004],_mm256_xor_si256(c2[1009],_mm256_xor_si256(c2[529],_mm256_xor_si256(c2[721],_mm256_xor_si256(c2[241],_mm256_xor_si256(c2[592],_mm256_xor_si256(c2[17],_mm256_xor_si256(c2[560],_mm256_xor_si256(c2[1013],_mm256_xor_si256(c2[533],_mm256_xor_si256(c2[916],_mm256_xor_si256(c2[341],_mm256_xor_si256(c2[884],_mm256_xor_si256(c2[628],_mm256_xor_si256(c2[24],_mm256_xor_si256(c2[1017],_mm256_xor_si256(c2[537],_mm256_xor_si256(c2[153],_mm256_xor_si256(c2[696],_mm256_xor_si256(c2[184],_mm256_xor_si256(c2[632],_mm256_xor_si256(c2[152],_mm256_xor_si256(c2[988],_mm256_xor_si256(c2[508],_mm256_xor_si256(c2[349],_mm256_xor_si256(c2[797],_mm256_xor_si256(c2[317],_mm256_xor_si256(c2[412],_mm256_xor_si256(c2[97],_mm256_xor_si256(c2[640],_mm256_xor_si256(c2[609],_mm256_xor_si256(c2[129],_mm256_xor_si256(c2[224],_mm256_xor_si256(c2[672],_mm256_xor_si256(c2[192],_mm256_xor_si256(c2[548],_mm256_xor_si256(c2[516],_mm256_xor_si256(c2[36],_mm256_xor_si256(c2[997],_mm256_xor_si256(c2[517],_mm256_xor_si256(c2[677],_mm256_xor_si256(c2[100],c2[645]))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))));

//row: 8
     d2[16]=_mm256_xor_si256(c2[481],_mm256_xor_si256(c2[449],_mm256_xor_si256(c2[416],_mm256_xor_si256(c2[384],_mm256_xor_si256(c2[705],_mm256_xor_si256(c2[448],_mm256_xor_si256(c2[868],_mm256_xor_si256(c2[836],_mm256_xor_si256(c2[740],_mm256_xor_si256(c2[421],_mm256_xor_si256(c2[964],_mm256_xor_si256(c2[585],_mm256_xor_si256(c2[553],_mm256_xor_si256(c2[1000],_mm256_xor_si256(c2[12],_mm256_xor_si256(c2[1005],_mm256_xor_si256(c2[396],_mm256_xor_si256(c2[364],_mm256_xor_si256(c2[556],_mm256_xor_si256(c2[113],_mm256_xor_si256(c2[81],_mm256_xor_si256(c2[816],_mm256_xor_si256(c2[112],_mm256_xor_si256(c2[117],_mm256_xor_si256(c2[85],_mm256_xor_si256(c2[436],_mm256_xor_si256(c2[121],_mm256_xor_si256(c2[89],_mm256_xor_si256(c2[280],_mm256_xor_si256(c2[248],_mm256_xor_si256(c2[729],_mm256_xor_si256(c2[92],_mm256_xor_si256(c2[60],_mm256_xor_si256(c2[892],_mm256_xor_si256(c2[224],_mm256_xor_si256(c2[192],_mm256_xor_si256(c2[704],_mm256_xor_si256(c2[769],_mm256_xor_si256(c2[645],_mm256_xor_si256(c2[613],_mm256_xor_si256(c2[101],_mm256_xor_si256(c2[69],c2[197]))))))))))))))))))))))))))))))))))))))))));

//row: 9
     d2[18]=_mm256_xor_si256(c2[0],_mm256_xor_si256(c2[160],_mm256_xor_si256(c2[993],_mm256_xor_si256(c2[97],_mm256_xor_si256(c2[928],_mm256_xor_si256(c2[416],_mm256_xor_si256(c2[224],_mm256_xor_si256(c2[389],_mm256_xor_si256(c2[549],_mm256_xor_si256(c2[357],_mm256_xor_si256(c2[453],_mm256_xor_si256(c2[261],_mm256_xor_si256(c2[132],_mm256_xor_si256(c2[965],_mm256_xor_si256(c2[356],_mm256_xor_si256(c2[104],_mm256_xor_si256(c2[264],_mm256_xor_si256(c2[72],_mm256_xor_si256(c2[713],_mm256_xor_si256(c2[521],_mm256_xor_si256(c2[556],_mm256_xor_si256(c2[716],_mm256_xor_si256(c2[524],_mm256_xor_si256(c2[77],_mm256_xor_si256(c2[908],_mm256_xor_si256(c2[269],_mm256_xor_si256(c2[77],_mm256_xor_si256(c2[817],_mm256_xor_si256(c2[625],_mm256_xor_si256(c2[529],_mm256_xor_si256(c2[337],_mm256_xor_si256(c2[848],_mm256_xor_si256(c2[656],_mm256_xor_si256(c2[821],_mm256_xor_si256(c2[629],_mm256_xor_si256(c2[149],_mm256_xor_si256(c2[980],_mm256_xor_si256(c2[665],_mm256_xor_si256(c2[825],_mm256_xor_si256(c2[633],_mm256_xor_si256(c2[984],_mm256_xor_si256(c2[792],_mm256_xor_si256(c2[440],_mm256_xor_si256(c2[248],_mm256_xor_si256(c2[796],_mm256_xor_si256(c2[604],_mm256_xor_si256(c2[605],_mm256_xor_si256(c2[413],_mm256_xor_si256(c2[928],_mm256_xor_si256(c2[736],_mm256_xor_si256(c2[417],_mm256_xor_si256(c2[225],_mm256_xor_si256(c2[480],_mm256_xor_si256(c2[288],_mm256_xor_si256(c2[448],_mm256_xor_si256(c2[164],_mm256_xor_si256(c2[324],_mm256_xor_si256(c2[132],_mm256_xor_si256(c2[805],_mm256_xor_si256(c2[613],_mm256_xor_si256(c2[933],c2[741])))))))))))))))))))))))))))))))))))))))))))))))))))))))))))));

//row: 10
     d2[20]=_mm256_xor_si256(c2[352],_mm256_xor_si256(c2[805],_mm256_xor_si256(c2[24],c2[701])));

//row: 11
     d2[22]=_mm256_xor_si256(c2[0],_mm256_xor_si256(c2[960],_mm256_xor_si256(c2[256],_mm256_xor_si256(c2[352],_mm256_xor_si256(c2[389],_mm256_xor_si256(c2[293],_mm256_xor_si256(c2[4],_mm256_xor_si256(c2[997],_mm256_xor_si256(c2[104],_mm256_xor_si256(c2[585],_mm256_xor_si256(c2[553],_mm256_xor_si256(c2[556],_mm256_xor_si256(c2[940],_mm256_xor_si256(c2[109],_mm256_xor_si256(c2[657],_mm256_xor_si256(c2[369],_mm256_xor_si256(c2[720],_mm256_xor_si256(c2[688],_mm256_xor_si256(c2[661],_mm256_xor_si256(c2[21],_mm256_xor_si256(c2[1012],_mm256_xor_si256(c2[665],_mm256_xor_si256(c2[824],_mm256_xor_si256(c2[312],_mm256_xor_si256(c2[280],_mm256_xor_si256(c2[636],_mm256_xor_si256(c2[477],_mm256_xor_si256(c2[445],_mm256_xor_si256(c2[413],_mm256_xor_si256(c2[768],_mm256_xor_si256(c2[257],_mm256_xor_si256(c2[352],_mm256_xor_si256(c2[320],_mm256_xor_si256(c2[164],_mm256_xor_si256(c2[645],_mm256_xor_si256(c2[805],_mm256_xor_si256(c2[773],c2[612])))))))))))))))))))))))))))))))))))));

//row: 12
     d2[24]=_mm256_xor_si256(c2[736],_mm256_xor_si256(c2[704],_mm256_xor_si256(c2[641],_mm256_xor_si256(c2[960],_mm256_xor_si256(c2[100],_mm256_xor_si256(c2[68],_mm256_xor_si256(c2[997],_mm256_xor_si256(c2[676],_mm256_xor_si256(c2[997],_mm256_xor_si256(c2[840],_mm256_xor_si256(c2[808],_mm256_xor_si256(c2[232],_mm256_xor_si256(c2[269],_mm256_xor_si256(c2[237],_mm256_xor_si256(c2[621],_mm256_xor_si256(c2[813],_mm256_xor_si256(c2[493],_mm256_xor_si256(c2[336],_mm256_xor_si256(c2[48],_mm256_xor_si256(c2[369],_mm256_xor_si256(c2[340],_mm256_xor_si256(c2[693],_mm256_xor_si256(c2[376],_mm256_xor_si256(c2[344],_mm256_xor_si256(c2[505],_mm256_xor_si256(c2[984],_mm256_xor_si256(c2[317],_mm256_xor_si256(c2[124],_mm256_xor_si256(c2[449],_mm256_xor_si256(c2[961],_mm256_xor_si256(c2[1024],_mm256_xor_si256(c2[900],_mm256_xor_si256(c2[868],_mm256_xor_si256(c2[324],c2[452]))))))))))))))))))))))))))))))))));

//row: 13
     d2[26]=_mm256_xor_si256(c2[193],_mm256_xor_si256(c2[128],_mm256_xor_si256(c2[449],_mm256_xor_si256(c2[608],_mm256_xor_si256(c2[580],_mm256_xor_si256(c2[484],_mm256_xor_si256(c2[197],_mm256_xor_si256(c2[165],_mm256_xor_si256(c2[68],_mm256_xor_si256(c2[297],_mm256_xor_si256(c2[776],_mm256_xor_si256(c2[744],_mm256_xor_si256(c2[749],_mm256_xor_si256(c2[108],_mm256_xor_si256(c2[300],_mm256_xor_si256(c2[848],_mm256_xor_si256(c2[560],_mm256_xor_si256(c2[913],_mm256_xor_si256(c2[881],_mm256_xor_si256(c2[852],_mm256_xor_si256(c2[212],_mm256_xor_si256(c2[180],_mm256_xor_si256(c2[856],_mm256_xor_si256(c2[1017],_mm256_xor_si256(c2[505],_mm256_xor_si256(c2[473],_mm256_xor_si256(c2[829],_mm256_xor_si256(c2[668],_mm256_xor_si256(c2[636],_mm256_xor_si256(c2[961],_mm256_xor_si256(c2[448],_mm256_xor_si256(c2[545],_mm256_xor_si256(c2[513],_mm256_xor_si256(c2[225],_mm256_xor_si256(c2[357],_mm256_xor_si256(c2[836],_mm256_xor_si256(c2[996],c2[964])))))))))))))))))))))))))))))))))))));

//row: 14
     d2[28]=_mm256_xor_si256(c2[384],_mm256_xor_si256(c2[352],_mm256_xor_si256(c2[513],_mm256_xor_si256(c2[289],_mm256_xor_si256(c2[448],_mm256_xor_si256(c2[608],_mm256_xor_si256(c2[769],_mm256_xor_si256(c2[773],_mm256_xor_si256(c2[741],_mm256_xor_si256(c2[900],_mm256_xor_si256(c2[645],_mm256_xor_si256(c2[804],_mm256_xor_si256(c2[517],_mm256_xor_si256(c2[324],_mm256_xor_si256(c2[485],_mm256_xor_si256(c2[613],_mm256_xor_si256(c2[488],_mm256_xor_si256(c2[456],_mm256_xor_si256(c2[617],_mm256_xor_si256(c2[73],_mm256_xor_si256(c2[905],_mm256_xor_si256(c2[41],_mm256_xor_si256(c2[940],_mm256_xor_si256(c2[908],_mm256_xor_si256(c2[44],_mm256_xor_si256(c2[269],_mm256_xor_si256(c2[428],_mm256_xor_si256(c2[461],_mm256_xor_si256(c2[620],_mm256_xor_si256(c2[1009],_mm256_xor_si256(c2[145],_mm256_xor_si256(c2[721],_mm256_xor_si256(c2[880],_mm256_xor_si256(c2[208],_mm256_xor_si256(c2[17],_mm256_xor_si256(c2[176],_mm256_xor_si256(c2[1013],_mm256_xor_si256(c2[149],_mm256_xor_si256(c2[532],_mm256_xor_si256(c2[341],_mm256_xor_si256(c2[500],_mm256_xor_si256(c2[24],_mm256_xor_si256(c2[1017],_mm256_xor_si256(c2[153],_mm256_xor_si256(c2[153],_mm256_xor_si256(c2[312],_mm256_xor_si256(c2[825],_mm256_xor_si256(c2[632],_mm256_xor_si256(c2[793],_mm256_xor_si256(c2[568],_mm256_xor_si256(c2[988],_mm256_xor_si256(c2[124],_mm256_xor_si256(c2[988],_mm256_xor_si256(c2[797],_mm256_xor_si256(c2[956],_mm256_xor_si256(c2[97],_mm256_xor_si256(c2[256],_mm256_xor_si256(c2[609],_mm256_xor_si256(c2[768],_mm256_xor_si256(c2[865],_mm256_xor_si256(c2[672],_mm256_xor_si256(c2[833],_mm256_xor_si256(c2[548],_mm256_xor_si256(c2[516],_mm256_xor_si256(c2[677],_mm256_xor_si256(c2[997],_mm256_xor_si256(c2[133],_mm256_xor_si256(c2[293],_mm256_xor_si256(c2[100],c2[261])))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))));

//row: 15
     d2[30]=_mm256_xor_si256(c2[960],_mm256_xor_si256(c2[737],_mm256_xor_si256(c2[928],_mm256_xor_si256(c2[672],_mm256_xor_si256(c2[865],_mm256_xor_si256(c2[993],_mm256_xor_si256(c2[161],_mm256_xor_si256(c2[609],_mm256_xor_si256(c2[324],_mm256_xor_si256(c2[101],_mm256_xor_si256(c2[292],_mm256_xor_si256(c2[5],_mm256_xor_si256(c2[196],_mm256_xor_si256(c2[709],_mm256_xor_si256(c2[900],_mm256_xor_si256(c2[41],_mm256_xor_si256(c2[841],_mm256_xor_si256(c2[9],_mm256_xor_si256(c2[265],_mm256_xor_si256(c2[456],_mm256_xor_si256(c2[493],_mm256_xor_si256(c2[268],_mm256_xor_si256(c2[461],_mm256_xor_si256(c2[652],_mm256_xor_si256(c2[845],_mm256_xor_si256(c2[844],_mm256_xor_si256(c2[12],_mm256_xor_si256(c2[369],_mm256_xor_si256(c2[560],_mm256_xor_si256(c2[81],_mm256_xor_si256(c2[272],_mm256_xor_si256(c2[400],_mm256_xor_si256(c2[593],_mm256_xor_si256(c2[373],_mm256_xor_si256(c2[564],_mm256_xor_si256(c2[724],_mm256_xor_si256(c2[917],_mm256_xor_si256(c2[600],_mm256_xor_si256(c2[377],_mm256_xor_si256(c2[568],_mm256_xor_si256(c2[536],_mm256_xor_si256(c2[729],_mm256_xor_si256(c2[1017],_mm256_xor_si256(c2[185],_mm256_xor_si256(c2[348],_mm256_xor_si256(c2[541],_mm256_xor_si256(c2[157],_mm256_xor_si256(c2[348],_mm256_xor_si256(c2[480],_mm256_xor_si256(c2[673],_mm256_xor_si256(c2[992],_mm256_xor_si256(c2[160],_mm256_xor_si256(c2[32],_mm256_xor_si256(c2[225],_mm256_xor_si256(c2[101],_mm256_xor_si256(c2[901],_mm256_xor_si256(c2[69],_mm256_xor_si256(c2[357],_mm256_xor_si256(c2[548],_mm256_xor_si256(c2[485],c2[676]))))))))))))))))))))))))))))))))))))))))))))))))))))))))))));

//row: 16
     d2[32]=_mm256_xor_si256(c2[544],_mm256_xor_si256(c2[865],_mm256_xor_si256(c2[512],_mm256_xor_si256(c2[833],_mm256_xor_si256(c2[800],_mm256_xor_si256(c2[449],_mm256_xor_si256(c2[768],_mm256_xor_si256(c2[768],_mm256_xor_si256(c2[64],_mm256_xor_si256(c2[933],_mm256_xor_si256(c2[229],_mm256_xor_si256(c2[901],_mm256_xor_si256(c2[197],_mm256_xor_si256(c2[805],_mm256_xor_si256(c2[101],_mm256_xor_si256(c2[484],_mm256_xor_si256(c2[805],_mm256_xor_si256(c2[356],_mm256_xor_si256(c2[648],_mm256_xor_si256(c2[969],_mm256_xor_si256(c2[616],_mm256_xor_si256(c2[937],_mm256_xor_si256(c2[40],_mm256_xor_si256(c2[361],_mm256_xor_si256(c2[77],_mm256_xor_si256(c2[396],_mm256_xor_si256(c2[45],_mm256_xor_si256(c2[364],_mm256_xor_si256(c2[780],_mm256_xor_si256(c2[429],_mm256_xor_si256(c2[748],_mm256_xor_si256(c2[621],_mm256_xor_si256(c2[940],_mm256_xor_si256(c2[497],_mm256_xor_si256(c2[144],_mm256_xor_si256(c2[465],_mm256_xor_si256(c2[881],_mm256_xor_si256(c2[177],_mm256_xor_si256(c2[177],_mm256_xor_si256(c2[496],_mm256_xor_si256(c2[501],_mm256_xor_si256(c2[148],_mm256_xor_si256(c2[469],_mm256_xor_si256(c2[501],_mm256_xor_si256(c2[820],_mm256_xor_si256(c2[184],_mm256_xor_si256(c2[505],_mm256_xor_si256(c2[152],_mm256_xor_si256(c2[473],_mm256_xor_si256(c2[664],_mm256_xor_si256(c2[313],_mm256_xor_si256(c2[632],_mm256_xor_si256(c2[792],_mm256_xor_si256(c2[88],_mm256_xor_si256(c2[476],_mm256_xor_si256(c2[125],_mm256_xor_si256(c2[444],_mm256_xor_si256(c2[957],_mm256_xor_si256(c2[253],_mm256_xor_si256(c2[608],_mm256_xor_si256(c2[257],_mm256_xor_si256(c2[576],_mm256_xor_si256(c2[769],_mm256_xor_si256(c2[65],_mm256_xor_si256(c2[832],_mm256_xor_si256(c2[128],_mm256_xor_si256(c2[708],_mm256_xor_si256(c2[1029],_mm256_xor_si256(c2[676],_mm256_xor_si256(c2[997],_mm256_xor_si256(c2[485],_mm256_xor_si256(c2[132],_mm256_xor_si256(c2[453],_mm256_xor_si256(c2[260],_mm256_xor_si256(c2[581],c2[484])))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))));

//row: 17
     d2[34]=_mm256_xor_si256(c2[865],_mm256_xor_si256(c2[288],_mm256_xor_si256(c2[833],_mm256_xor_si256(c2[256],_mm256_xor_si256(c2[225],_mm256_xor_si256(c2[768],_mm256_xor_si256(c2[193],_mm256_xor_si256(c2[64],_mm256_xor_si256(c2[512],_mm256_xor_si256(c2[229],_mm256_xor_si256(c2[677],_mm256_xor_si256(c2[197],_mm256_xor_si256(c2[645],_mm256_xor_si256(c2[101],_mm256_xor_si256(c2[549],_mm256_xor_si256(c2[805],_mm256_xor_si256(c2[228],_mm256_xor_si256(c2[965],_mm256_xor_si256(c2[969],_mm256_xor_si256(c2[392],_mm256_xor_si256(c2[937],_mm256_xor_si256(c2[360],_mm256_xor_si256(c2[361],_mm256_xor_si256(c2[809],_mm256_xor_si256(c2[396],_mm256_xor_si256(c2[844],_mm256_xor_si256(c2[364],_mm256_xor_si256(c2[812],_mm256_xor_si256(c2[205],_mm256_xor_si256(c2[748],_mm256_xor_si256(c2[173],_mm256_xor_si256(c2[940],_mm256_xor_si256(c2[365],_mm256_xor_si256(c2[945],_mm256_xor_si256(c2[465],_mm256_xor_si256(c2[913],_mm256_xor_si256(c2[177],_mm256_xor_si256(c2[625],_mm256_xor_si256(c2[496],_mm256_xor_si256(c2[944],_mm256_xor_si256(c2[949],_mm256_xor_si256(c2[469],_mm256_xor_si256(c2[917],_mm256_xor_si256(c2[820],_mm256_xor_si256(c2[245],_mm256_xor_si256(c2[917],_mm256_xor_si256(c2[505],_mm256_xor_si256(c2[953],_mm256_xor_si256(c2[473],_mm256_xor_si256(c2[921],_mm256_xor_si256(c2[89],_mm256_xor_si256(c2[632],_mm256_xor_si256(c2[57],_mm256_xor_si256(c2[88],_mm256_xor_si256(c2[536],_mm256_xor_si256(c2[924],_mm256_xor_si256(c2[444],_mm256_xor_si256(c2[892],_mm256_xor_si256(c2[253],_mm256_xor_si256(c2[701],_mm256_xor_si256(c2[33],_mm256_xor_si256(c2[576],_mm256_xor_si256(c2[1024],_mm256_xor_si256(c2[65],_mm256_xor_si256(c2[513],_mm256_xor_si256(c2[128],_mm256_xor_si256(c2[576],_mm256_xor_si256(c2[1029],_mm256_xor_si256(c2[452],_mm256_xor_si256(c2[997],_mm256_xor_si256(c2[420],_mm256_xor_si256(c2[933],_mm256_xor_si256(c2[453],_mm256_xor_si256(c2[901],_mm256_xor_si256(c2[581],c2[1029])))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))));

//row: 18
     d2[36]=_mm256_xor_si256(c2[896],_mm256_xor_si256(c2[88],c2[605]));

//row: 19
     d2[38]=_mm256_xor_si256(c2[32],_mm256_xor_si256(c2[992],_mm256_xor_si256(c2[288],_mm256_xor_si256(c2[736],_mm256_xor_si256(c2[421],_mm256_xor_si256(c2[325],_mm256_xor_si256(c2[4],_mm256_xor_si256(c2[644],_mm256_xor_si256(c2[136],_mm256_xor_si256(c2[585],_mm256_xor_si256(c2[588],_mm256_xor_si256(c2[972],_mm256_xor_si256(c2[141],_mm256_xor_si256(c2[689],_mm256_xor_si256(c2[401],_mm256_xor_si256(c2[720],_mm256_xor_si256(c2[693],_mm256_xor_si256(c2[21],_mm256_xor_si256(c2[697],_mm256_xor_si256(c2[856],_mm256_xor_si256(c2[312],_mm256_xor_si256(c2[668],_mm256_xor_si256(c2[477],_mm256_xor_si256(c2[800],_mm256_xor_si256(c2[289],_mm256_xor_si256(c2[352],_mm256_xor_si256(c2[196],_mm256_xor_si256(c2[677],c2[805]))))))))))))))))))))))))))));

//row: 20
     d2[40]=_mm256_xor_si256(c2[193],_mm256_xor_si256(c2[161],_mm256_xor_si256(c2[96],_mm256_xor_si256(c2[417],_mm256_xor_si256(c2[580],_mm256_xor_si256(c2[548],_mm256_xor_si256(c2[452],_mm256_xor_si256(c2[133],_mm256_xor_si256(c2[836],_mm256_xor_si256(c2[297],_mm256_xor_si256(c2[265],_mm256_xor_si256(c2[712],_mm256_xor_si256(c2[749],_mm256_xor_si256(c2[717],_mm256_xor_si256(c2[76],_mm256_xor_si256(c2[268],_mm256_xor_si256(c2[816],_mm256_xor_si256(c2[528],_mm256_xor_si256(c2[849],_mm256_xor_si256(c2[305],_mm256_xor_si256(c2[820],_mm256_xor_si256(c2[148],_mm256_xor_si256(c2[856],_mm256_xor_si256(c2[824],_mm256_xor_si256(c2[985],_mm256_xor_si256(c2[441],_mm256_xor_si256(c2[797],_mm256_xor_si256(c2[604],_mm256_xor_si256(c2[929],_mm256_xor_si256(c2[416],_mm256_xor_si256(c2[481],_mm256_xor_si256(c2[357],_mm256_xor_si256(c2[325],_mm256_xor_si256(c2[804],c2[932]))))))))))))))))))))))))))))))))));

//row: 21
     d2[42]=_mm256_xor_si256(c2[832],_mm256_xor_si256(c2[769],_mm256_xor_si256(c2[65],_mm256_xor_si256(c2[384],_mm256_xor_si256(c2[196],_mm256_xor_si256(c2[100],_mm256_xor_si256(c2[836],_mm256_xor_si256(c2[804],_mm256_xor_si256(c2[936],_mm256_xor_si256(c2[392],_mm256_xor_si256(c2[360],_mm256_xor_si256(c2[365],_mm256_xor_si256(c2[749],_mm256_xor_si256(c2[941],_mm256_xor_si256(c2[464],_mm256_xor_si256(c2[176],_mm256_xor_si256(c2[529],_mm256_xor_si256(c2[497],_mm256_xor_si256(c2[468],_mm256_xor_si256(c2[853],_mm256_xor_si256(c2[821],_mm256_xor_si256(c2[472],_mm256_xor_si256(c2[633],_mm256_xor_si256(c2[121],_mm256_xor_si256(c2[89],_mm256_xor_si256(c2[445],_mm256_xor_si256(c2[284],_mm256_xor_si256(c2[252],_mm256_xor_si256(c2[577],_mm256_xor_si256(c2[64],_mm256_xor_si256(c2[161],_mm256_xor_si256(c2[129],_mm256_xor_si256(c2[353],_mm256_xor_si256(c2[996],_mm256_xor_si256(c2[452],_mm256_xor_si256(c2[612],c2[580]))))))))))))))))))))))))))))))))))));

//row: 22
     d2[44]=_mm256_xor_si256(c2[964],c2[1001]);

//row: 23
     d2[46]=_mm256_xor_si256(c2[736],_mm256_xor_si256(c2[365],c2[469]));

//row: 24
     d2[48]=_mm256_xor_si256(c2[453],_mm256_xor_si256(c2[360],c2[292]));

//row: 25
     d2[50]=_mm256_xor_si256(c2[129],c2[916]);

//row: 26
     d2[52]=_mm256_xor_si256(c2[289],_mm256_xor_si256(c2[257],_mm256_xor_si256(c2[64],_mm256_xor_si256(c2[224],_mm256_xor_si256(c2[192],_mm256_xor_si256(c2[1],_mm256_xor_si256(c2[513],_mm256_xor_si256(c2[320],_mm256_xor_si256(c2[676],_mm256_xor_si256(c2[644],_mm256_xor_si256(c2[453],_mm256_xor_si256(c2[548],_mm256_xor_si256(c2[357],_mm256_xor_si256(c2[68],_mm256_xor_si256(c2[229],_mm256_xor_si256(c2[36],_mm256_xor_si256(c2[393],_mm256_xor_si256(c2[361],_mm256_xor_si256(c2[168],_mm256_xor_si256(c2[649],_mm256_xor_si256(c2[808],_mm256_xor_si256(c2[617],_mm256_xor_si256(c2[936],_mm256_xor_si256(c2[845],_mm256_xor_si256(c2[813],_mm256_xor_si256(c2[620],_mm256_xor_si256(c2[204],_mm256_xor_si256(c2[172],_mm256_xor_si256(c2[1004],_mm256_xor_si256(c2[364],_mm256_xor_si256(c2[173],_mm256_xor_si256(c2[944],_mm256_xor_si256(c2[912],_mm256_xor_si256(c2[721],_mm256_xor_si256(c2[624],_mm256_xor_si256(c2[433],_mm256_xor_si256(c2[784],_mm256_xor_si256(c2[945],_mm256_xor_si256(c2[752],_mm256_xor_si256(c2[948],_mm256_xor_si256(c2[916],_mm256_xor_si256(c2[725],_mm256_xor_si256(c2[85],_mm256_xor_si256(c2[244],_mm256_xor_si256(c2[53],_mm256_xor_si256(c2[952],_mm256_xor_si256(c2[920],_mm256_xor_si256(c2[729],_mm256_xor_si256(c2[88],_mm256_xor_si256(c2[56],_mm256_xor_si256(c2[888],_mm256_xor_si256(c2[376],_mm256_xor_si256(c2[537],_mm256_xor_si256(c2[344],_mm256_xor_si256(c2[925],_mm256_xor_si256(c2[893],_mm256_xor_si256(c2[700],_mm256_xor_si256(c2[541],_mm256_xor_si256(c2[700],_mm256_xor_si256(c2[509],_mm256_xor_si256(c2[508],_mm256_xor_si256(c2[32],_mm256_xor_si256(c2[1025],_mm256_xor_si256(c2[832],_mm256_xor_si256(c2[512],_mm256_xor_si256(c2[321],_mm256_xor_si256(c2[416],_mm256_xor_si256(c2[577],_mm256_xor_si256(c2[384],_mm256_xor_si256(c2[453],_mm256_xor_si256(c2[421],_mm256_xor_si256(c2[228],_mm256_xor_si256(c2[932],_mm256_xor_si256(c2[900],_mm256_xor_si256(c2[709],_mm256_xor_si256(c2[869],_mm256_xor_si256(c2[1028],c2[837])))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))));

//row: 27
     d2[54]=_mm256_xor_si256(c2[256],c2[760]);

//row: 28
     d2[56]=_mm256_xor_si256(c2[69],_mm256_xor_si256(c2[169],c2[244]));

//row: 29
     d2[58]=_mm256_xor_si256(c2[576],c2[912]);

//row: 30
     d2[60]=_mm256_xor_si256(c2[232],_mm256_xor_si256(c2[533],_mm256_xor_si256(c2[316],c2[676])));

//row: 31
     d2[62]=_mm256_xor_si256(c2[288],_mm256_xor_si256(c2[225],_mm256_xor_si256(c2[544],_mm256_xor_si256(c2[677],_mm256_xor_si256(c2[581],_mm256_xor_si256(c2[292],_mm256_xor_si256(c2[260],_mm256_xor_si256(c2[325],_mm256_xor_si256(c2[392],_mm256_xor_si256(c2[873],_mm256_xor_si256(c2[841],_mm256_xor_si256(c2[844],_mm256_xor_si256(c2[205],_mm256_xor_si256(c2[397],_mm256_xor_si256(c2[945],_mm256_xor_si256(c2[657],_mm256_xor_si256(c2[1008],_mm256_xor_si256(c2[976],_mm256_xor_si256(c2[949],_mm256_xor_si256(c2[309],_mm256_xor_si256(c2[277],_mm256_xor_si256(c2[953],_mm256_xor_si256(c2[89],_mm256_xor_si256(c2[600],_mm256_xor_si256(c2[568],_mm256_xor_si256(c2[924],_mm256_xor_si256(c2[765],_mm256_xor_si256(c2[733],_mm256_xor_si256(c2[33],_mm256_xor_si256(c2[545],_mm256_xor_si256(c2[640],_mm256_xor_si256(c2[608],_mm256_xor_si256(c2[452],_mm256_xor_si256(c2[933],_mm256_xor_si256(c2[68],c2[36])))))))))))))))))))))))))))))))))));

//row: 32
     d2[64]=_mm256_xor_si256(c2[481],_mm256_xor_si256(c2[449],_mm256_xor_si256(c2[416],_mm256_xor_si256(c2[384],_mm256_xor_si256(c2[705],_mm256_xor_si256(c2[577],_mm256_xor_si256(c2[868],_mm256_xor_si256(c2[836],_mm256_xor_si256(c2[740],_mm256_xor_si256(c2[421],_mm256_xor_si256(c2[585],_mm256_xor_si256(c2[553],_mm256_xor_si256(c2[1000],_mm256_xor_si256(c2[12],_mm256_xor_si256(c2[1005],_mm256_xor_si256(c2[396],_mm256_xor_si256(c2[364],_mm256_xor_si256(c2[556],_mm256_xor_si256(c2[113],_mm256_xor_si256(c2[81],_mm256_xor_si256(c2[816],_mm256_xor_si256(c2[112],_mm256_xor_si256(c2[117],_mm256_xor_si256(c2[85],_mm256_xor_si256(c2[436],_mm256_xor_si256(c2[405],_mm256_xor_si256(c2[121],_mm256_xor_si256(c2[89],_mm256_xor_si256(c2[280],_mm256_xor_si256(c2[248],_mm256_xor_si256(c2[729],_mm256_xor_si256(c2[92],_mm256_xor_si256(c2[60],_mm256_xor_si256(c2[892],_mm256_xor_si256(c2[224],_mm256_xor_si256(c2[192],_mm256_xor_si256(c2[704],_mm256_xor_si256(c2[769],_mm256_xor_si256(c2[645],_mm256_xor_si256(c2[613],_mm256_xor_si256(c2[101],_mm256_xor_si256(c2[69],c2[197]))))))))))))))))))))))))))))))))))))))))));

//row: 33
     d2[66]=_mm256_xor_si256(c2[609],_mm256_xor_si256(c2[544],_mm256_xor_si256(c2[865],_mm256_xor_si256(c2[996],_mm256_xor_si256(c2[900],_mm256_xor_si256(c2[581],_mm256_xor_si256(c2[713],_mm256_xor_si256(c2[137],_mm256_xor_si256(c2[136],_mm256_xor_si256(c2[140],_mm256_xor_si256(c2[524],_mm256_xor_si256(c2[716],_mm256_xor_si256(c2[241],_mm256_xor_si256(c2[976],_mm256_xor_si256(c2[272],_mm256_xor_si256(c2[245],_mm256_xor_si256(c2[596],_mm256_xor_si256(c2[249],_mm256_xor_si256(c2[408],_mm256_xor_si256(c2[889],_mm256_xor_si256(c2[220],_mm256_xor_si256(c2[29],_mm256_xor_si256(c2[157],_mm256_xor_si256(c2[352],_mm256_xor_si256(c2[864],_mm256_xor_si256(c2[929],_mm256_xor_si256(c2[773],_mm256_xor_si256(c2[229],c2[357]))))))))))))))))))))))))))));

//row: 34
     d2[68]=_mm256_xor_si256(c2[960],_mm256_xor_si256(c2[928],_mm256_xor_si256(c2[385],_mm256_xor_si256(c2[897],_mm256_xor_si256(c2[865],_mm256_xor_si256(c2[320],_mm256_xor_si256(c2[161],_mm256_xor_si256(c2[641],_mm256_xor_si256(c2[608],_mm256_xor_si256(c2[324],_mm256_xor_si256(c2[292],_mm256_xor_si256(c2[772],_mm256_xor_si256(c2[196],_mm256_xor_si256(c2[676],_mm256_xor_si256(c2[389],_mm256_xor_si256(c2[900],_mm256_xor_si256(c2[357],_mm256_xor_si256(c2[41],_mm256_xor_si256(c2[9],_mm256_xor_si256(c2[489],_mm256_xor_si256(c2[968],_mm256_xor_si256(c2[456],_mm256_xor_si256(c2[936],_mm256_xor_si256(c2[493],_mm256_xor_si256(c2[461],_mm256_xor_si256(c2[941],_mm256_xor_si256(c2[877],_mm256_xor_si256(c2[845],_mm256_xor_si256(c2[300],_mm256_xor_si256(c2[12],_mm256_xor_si256(c2[492],_mm256_xor_si256(c2[592],_mm256_xor_si256(c2[560],_mm256_xor_si256(c2[17],_mm256_xor_si256(c2[272],_mm256_xor_si256(c2[752],_mm256_xor_si256(c2[80],_mm256_xor_si256(c2[593],_mm256_xor_si256(c2[48],_mm256_xor_si256(c2[596],_mm256_xor_si256(c2[564],_mm256_xor_si256(c2[21],_mm256_xor_si256(c2[404],_mm256_xor_si256(c2[917],_mm256_xor_si256(c2[372],_mm256_xor_si256(c2[600],_mm256_xor_si256(c2[568],_mm256_xor_si256(c2[25],_mm256_xor_si256(c2[761],_mm256_xor_si256(c2[729],_mm256_xor_si256(c2[184],_mm256_xor_si256(c2[697],_mm256_xor_si256(c2[185],_mm256_xor_si256(c2[665],_mm256_xor_si256(c2[573],_mm256_xor_si256(c2[541],_mm256_xor_si256(c2[1021],_mm256_xor_si256(c2[860],_mm256_xor_si256(c2[348],_mm256_xor_si256(c2[828],_mm256_xor_si256(c2[705],_mm256_xor_si256(c2[673],_mm256_xor_si256(c2[128],_mm256_xor_si256(c2[160],_mm256_xor_si256(c2[640],_mm256_xor_si256(c2[737],_mm256_xor_si256(c2[225],_mm256_xor_si256(c2[705],_mm256_xor_si256(c2[101],_mm256_xor_si256(c2[69],_mm256_xor_si256(c2[549],_mm256_xor_si256(c2[580],_mm256_xor_si256(c2[548],_mm256_xor_si256(c2[1028],_mm256_xor_si256(c2[165],_mm256_xor_si256(c2[676],c2[133]))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))));

//row: 35
     d2[70]=_mm256_xor_si256(c2[256],_mm256_xor_si256(c2[224],_mm256_xor_si256(c2[161],_mm256_xor_si256(c2[480],_mm256_xor_si256(c2[645],_mm256_xor_si256(c2[613],_mm256_xor_si256(c2[517],_mm256_xor_si256(c2[196],_mm256_xor_si256(c2[805],_mm256_xor_si256(c2[360],_mm256_xor_si256(c2[328],_mm256_xor_si256(c2[777],_mm256_xor_si256(c2[812],_mm256_xor_si256(c2[780],_mm256_xor_si256(c2[141],_mm256_xor_si256(c2[333],_mm256_xor_si256(c2[881],_mm256_xor_si256(c2[593],_mm256_xor_si256(c2[912],_mm256_xor_si256(c2[885],_mm256_xor_si256(c2[213],_mm256_xor_si256(c2[277],_mm256_xor_si256(c2[921],_mm256_xor_si256(c2[889],_mm256_xor_si256(c2[25],_mm256_xor_si256(c2[504],_mm256_xor_si256(c2[860],_mm256_xor_si256(c2[669],_mm256_xor_si256(c2[992],_mm256_xor_si256(c2[481],_mm256_xor_si256(c2[544],_mm256_xor_si256(c2[420],_mm256_xor_si256(c2[388],_mm256_xor_si256(c2[869],c2[997]))))))))))))))))))))))))))))))))));

//row: 36
     d2[72]=_mm256_xor_si256(c2[384],_mm256_xor_si256(c2[201],c2[860]));

//row: 37
     d2[74]=_mm256_xor_si256(c2[97],_mm256_xor_si256(c2[992],_mm256_xor_si256(c2[32],_mm256_xor_si256(c2[929],_mm256_xor_si256(c2[353],_mm256_xor_si256(c2[225],_mm256_xor_si256(c2[484],_mm256_xor_si256(c2[356],_mm256_xor_si256(c2[388],_mm256_xor_si256(c2[260],_mm256_xor_si256(c2[996],_mm256_xor_si256(c2[69],_mm256_xor_si256(c2[964],_mm256_xor_si256(c2[201],_mm256_xor_si256(c2[73],_mm256_xor_si256(c2[552],_mm256_xor_si256(c2[648],_mm256_xor_si256(c2[520],_mm256_xor_si256(c2[653],_mm256_xor_si256(c2[525],_mm256_xor_si256(c2[12],_mm256_xor_si256(c2[909],_mm256_xor_si256(c2[204],_mm256_xor_si256(c2[76],_mm256_xor_si256(c2[752],_mm256_xor_si256(c2[624],_mm256_xor_si256(c2[464],_mm256_xor_si256(c2[336],_mm256_xor_si256(c2[689],_mm256_xor_si256(c2[785],_mm256_xor_si256(c2[657],_mm256_xor_si256(c2[756],_mm256_xor_si256(c2[628],_mm256_xor_si256(c2[1013],_mm256_xor_si256(c2[84],_mm256_xor_si256(c2[981],_mm256_xor_si256(c2[760],_mm256_xor_si256(c2[632],_mm256_xor_si256(c2[921],_mm256_xor_si256(c2[793],_mm256_xor_si256(c2[281],_mm256_xor_si256(c2[377],_mm256_xor_si256(c2[249],_mm256_xor_si256(c2[733],_mm256_xor_si256(c2[605],_mm256_xor_si256(c2[444],_mm256_xor_si256(c2[540],_mm256_xor_si256(c2[412],_mm256_xor_si256(c2[865],_mm256_xor_si256(c2[737],_mm256_xor_si256(c2[352],_mm256_xor_si256(c2[224],_mm256_xor_si256(c2[321],_mm256_xor_si256(c2[417],_mm256_xor_si256(c2[289],_mm256_xor_si256(c2[261],_mm256_xor_si256(c2[133],_mm256_xor_si256(c2[740],_mm256_xor_si256(c2[612],_mm256_xor_si256(c2[772],_mm256_xor_si256(c2[868],c2[740])))))))))))))))))))))))))))))))))))))))))))))))))))))))))))));

//row: 38
     d2[76]=_mm256_xor_si256(c2[481],_mm256_xor_si256(c2[449],_mm256_xor_si256(c2[384],_mm256_xor_si256(c2[705],_mm256_xor_si256(c2[868],_mm256_xor_si256(c2[836],_mm256_xor_si256(c2[740],_mm256_xor_si256(c2[421],_mm256_xor_si256(c2[996],_mm256_xor_si256(c2[585],_mm256_xor_si256(c2[553],_mm256_xor_si256(c2[1000],_mm256_xor_si256(c2[12],_mm256_xor_si256(c2[1005],_mm256_xor_si256(c2[364],_mm256_xor_si256(c2[556],_mm256_xor_si256(c2[81],_mm256_xor_si256(c2[816],_mm256_xor_si256(c2[112],_mm256_xor_si256(c2[85],_mm256_xor_si256(c2[436],_mm256_xor_si256(c2[84],_mm256_xor_si256(c2[121],_mm256_xor_si256(c2[89],_mm256_xor_si256(c2[248],_mm256_xor_si256(c2[729],_mm256_xor_si256(c2[60],_mm256_xor_si256(c2[892],_mm256_xor_si256(c2[192],_mm256_xor_si256(c2[704],_mm256_xor_si256(c2[769],_mm256_xor_si256(c2[645],_mm256_xor_si256(c2[613],_mm256_xor_si256(c2[69],c2[197]))))))))))))))))))))))))))))))))));

//row: 39
     d2[78]=_mm256_xor_si256(c2[353],_mm256_xor_si256(c2[321],_mm256_xor_si256(c2[288],_mm256_xor_si256(c2[256],_mm256_xor_si256(c2[577],_mm256_xor_si256(c2[481],_mm256_xor_si256(c2[740],_mm256_xor_si256(c2[708],_mm256_xor_si256(c2[612],_mm256_xor_si256(c2[293],_mm256_xor_si256(c2[457],_mm256_xor_si256(c2[425],_mm256_xor_si256(c2[872],_mm256_xor_si256(c2[909],_mm256_xor_si256(c2[877],_mm256_xor_si256(c2[268],_mm256_xor_si256(c2[236],_mm256_xor_si256(c2[428],_mm256_xor_si256(c2[1008],_mm256_xor_si256(c2[976],_mm256_xor_si256(c2[688],_mm256_xor_si256(c2[1009],_mm256_xor_si256(c2[1012],_mm256_xor_si256(c2[980],_mm256_xor_si256(c2[308],_mm256_xor_si256(c2[1016],_mm256_xor_si256(c2[984],_mm256_xor_si256(c2[152],_mm256_xor_si256(c2[120],_mm256_xor_si256(c2[601],_mm256_xor_si256(c2[989],_mm256_xor_si256(c2[957],_mm256_xor_si256(c2[764],_mm256_xor_si256(c2[413],_mm256_xor_si256(c2[96],_mm256_xor_si256(c2[64],_mm256_xor_si256(c2[576],_mm256_xor_si256(c2[641],_mm256_xor_si256(c2[517],_mm256_xor_si256(c2[485],_mm256_xor_si256(c2[996],_mm256_xor_si256(c2[964],c2[69]))))))))))))))))))))))))))))))))))))))))));

//row: 40
     d2[80]=_mm256_xor_si256(c2[608],_mm256_xor_si256(c2[0],_mm256_xor_si256(c2[545],_mm256_xor_si256(c2[960],_mm256_xor_si256(c2[864],_mm256_xor_si256(c2[256],_mm256_xor_si256(c2[997],_mm256_xor_si256(c2[389],_mm256_xor_si256(c2[901],_mm256_xor_si256(c2[293],_mm256_xor_si256(c2[4],_mm256_xor_si256(c2[580],_mm256_xor_si256(c2[997],_mm256_xor_si256(c2[712],_mm256_xor_si256(c2[104],_mm256_xor_si256(c2[585],_mm256_xor_si256(c2[136],_mm256_xor_si256(c2[553],_mm256_xor_si256(c2[8],_mm256_xor_si256(c2[141],_mm256_xor_si256(c2[556],_mm256_xor_si256(c2[525],_mm256_xor_si256(c2[940],_mm256_xor_si256(c2[717],_mm256_xor_si256(c2[109],_mm256_xor_si256(c2[240],_mm256_xor_si256(c2[657],_mm256_xor_si256(c2[977],_mm256_xor_si256(c2[369],_mm256_xor_si256(c2[720],_mm256_xor_si256(c2[273],_mm256_xor_si256(c2[688],_mm256_xor_si256(c2[244],_mm256_xor_si256(c2[661],_mm256_xor_si256(c2[21],_mm256_xor_si256(c2[597],_mm256_xor_si256(c2[1012],_mm256_xor_si256(c2[248],_mm256_xor_si256(c2[665],_mm256_xor_si256(c2[409],_mm256_xor_si256(c2[824],_mm256_xor_si256(c2[312],_mm256_xor_si256(c2[888],_mm256_xor_si256(c2[280],_mm256_xor_si256(c2[221],_mm256_xor_si256(c2[636],_mm256_xor_si256(c2[477],_mm256_xor_si256(c2[28],_mm256_xor_si256(c2[445],_mm256_xor_si256(c2[353],_mm256_xor_si256(c2[768],_mm256_xor_si256(c2[865],_mm256_xor_si256(c2[257],_mm256_xor_si256(c2[352],_mm256_xor_si256(c2[928],_mm256_xor_si256(c2[320],_mm256_xor_si256(c2[772],_mm256_xor_si256(c2[164],_mm256_xor_si256(c2[228],_mm256_xor_si256(c2[645],_mm256_xor_si256(c2[805],_mm256_xor_si256(c2[356],c2[773]))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))));

//row: 41
     d2[82]=_mm256_xor_si256(c2[993],_mm256_xor_si256(c2[961],_mm256_xor_si256(c2[896],_mm256_xor_si256(c2[192],_mm256_xor_si256(c2[357],_mm256_xor_si256(c2[325],_mm256_xor_si256(c2[229],_mm256_xor_si256(c2[933],_mm256_xor_si256(c2[36],_mm256_xor_si256(c2[72],_mm256_xor_si256(c2[40],_mm256_xor_si256(c2[489],_mm256_xor_si256(c2[524],_mm256_xor_si256(c2[492],_mm256_xor_si256(c2[876],_mm256_xor_si256(c2[45],_mm256_xor_si256(c2[593],_mm256_xor_si256(c2[305],_mm256_xor_si256(c2[624],_mm256_xor_si256(c2[597],_mm256_xor_si256(c2[948],_mm256_xor_si256(c2[181],_mm256_xor_si256(c2[633],_mm256_xor_si256(c2[601],_mm256_xor_si256(c2[760],_mm256_xor_si256(c2[216],_mm256_xor_si256(c2[572],_mm256_xor_si256(c2[381],_mm256_xor_si256(c2[704],_mm256_xor_si256(c2[193],_mm256_xor_si256(c2[256],_mm256_xor_si256(c2[132],_mm256_xor_si256(c2[100],_mm256_xor_si256(c2[581],c2[709]))))))))))))))))))))))))))))))))));
  }
}
