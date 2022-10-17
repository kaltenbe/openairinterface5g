import { Component } from '@angular/core';
import { UntypedFormArray } from '@angular/forms';
import { BehaviorSubject, forkJoin, timer } from 'rxjs';
import { Observable } from 'rxjs';
import { of } from 'rxjs';
import { filter } from 'rxjs/operators';
import { map } from 'rxjs/operators';
import { tap } from 'rxjs/operators';
import { switchMap } from 'rxjs/operators';
import { CommandsApi, IArgType, IColumn, ICommand, IInfo, ILogLvl, IParam, IRow } from 'src/app/api/commands.api';
import { CmdCtrl } from 'src/app/controls/cmd.control';
import { InfoCtrl } from 'src/app/controls/info.control';
import { ModuleCtrl } from 'src/app/controls/module.control';
import { RowCtrl } from 'src/app/controls/row.control';
import { VarCtrl } from 'src/app/controls/var.control';
import { DialogService } from 'src/app/services/dialog.service';
import { DownloadService } from 'src/app/services/download.service';
import { LoadingService } from 'src/app/services/loading.service';
import { ViewEncapsulation } from '@angular/core';

const CHANNEL_MOD_MODULE = "channelmod"
const PREDEF_CMD = "show predef"

@Component({
  selector: 'app-commands',
  templateUrl: './commands.component.html',
  styleUrls: ['./commands.component.scss'],
  encapsulation: ViewEncapsulation.None,
})
export class CommandsComponent {

  IArgType = IArgType;
  logLvlValues = Object.values(ILogLvl)

  // softmodem
  infos$: Observable<VarCtrl[]>
  modules$: Observable<ModuleCtrl[]>

  // module
  selectedModule?: ModuleCtrl
  vars$?: Observable<VarCtrl[]>
  cmds$?: Observable<CmdCtrl[]>

  // command
  selectedCmd?: ICommand
  displayedColumns: string[] = []
  rows$: BehaviorSubject<RowCtrl[]> = new BehaviorSubject<RowCtrl[]>([])
  columns: IColumn[] = []

  constructor(
    public commandsApi: CommandsApi,
    public loadingService: LoadingService,
    public dialogService: DialogService,
    public downloadService: DownloadService,

  ) {
    this.infos$ = this.commandsApi.readInfos$().pipe(
      map((infos) => infos.map(info => new InfoCtrl(info)))
    );

    this.modules$ = this.commandsApi.readModules$().pipe(
      map(imodules => imodules.map(imodule => new ModuleCtrl(imodule))),
      filter(controls => controls.length > 0),
      tap(controls => this.onModuleSelect(controls[0]))
    );
  }

  // get types$() {
  //   return this.modules$.pipe(
  //     filter(modules => modules.map(module => module.name).includes(CHANNEL_MOD_MODULE)),
  //     mergeMap(modules => this.commandsApi.readCommands$(modules[0]!.name)),
  //     map(icmds => icmds.filter(cmd => cmd.name === PREDEF_CMD)),
  //     filter(icmds => icmds.length > 0),
  //     map(icmds => new CmdCtrl(icmds[0])),
  //     mergeMap(control => this.commandsApi.runCommand$(control.api(), CHANNEL_MOD_MODULE)),
  //     map(resp => resp.display.map(line => line.match('/\s*[0-9]*\s*(\S*)\n/gm')![0])),
  //     tap(types => console.log(types.join(', ')))
  //   );
  // }

  onInfoSubmit(control: InfoCtrl) {
    let info: IInfo = control.api();

    if (info.type === IArgType.configfile) {
      this.downloadService.getFile(info.value)
    } else {
      this.commandsApi.setInfo$(info).subscribe();
    }
  }

  onModuleSelect(module: ModuleCtrl) {

    this.selectedModule = module
    this.selectedCmd = undefined

    this.cmds$ = this.commandsApi.readCommands$(module.name).pipe(
      map(icmds => icmds.map(icmd => new CmdCtrl(icmd))),
      map(cmds => {
        module.cmdsFA = new UntypedFormArray(cmds)
        return module.cmdsFA.controls as CmdCtrl[]
      })
    )

    this.vars$ = this.commandsApi.readVariables$(module.name).pipe(
      map(ivars => ivars.map(ivar => new VarCtrl(ivar))),
      map(vars => {
        module.varsFA = new UntypedFormArray(vars)
        return module.varsFA.controls as VarCtrl[]
      })
    )
  }

  onVarsubmit(control: VarCtrl) {
    this.commandsApi.setCmdVariable$(control.api(), this.selectedModule!.name)
      .pipe(
        map(resp => this.dialogService.openVarRespDialog(resp))
      ).subscribe();
  }

  onCmdSubmit(control: CmdCtrl) {

    this.selectedCmd = control.api()

    const obsparam$ = forkJoin([control.confirm
      ? this.dialogService.openConfirmDialog(control.confirm)
      : control.question
        ? this.dialogService.openQuestionDialog(this.selectedModule! + " " + this.selectedModule!.name, control)
        : of(true)]);

    obsparam$.pipe(
      switchMap(results => {
        if (!results[0])
          return of(null);
 
        return this.execCmd$(control);
      }))
      .subscribe();
  }

  private execCmd$(control: CmdCtrl) {

    this.commandsApi.runCommand$(control!.api(), this.selectedModule!.name).subscribe(resp => {
      if (resp.display[0]) this.dialogService.updateCmdDialog(control, resp, 'cmd ' + control.nameFC.value + ' response:')
      //          else return of(resp)

      const controls: RowCtrl[] = []
      this.displayedColumns = []

      if (resp.table) {
        this.columns = resp.table.columns
        this.displayedColumns = this.columns.map(col => col.name)
        this.displayedColumns.push('button')

        for (let rawIndex = 0; rawIndex < resp.table.rows.length; rawIndex++) {

          let params: IParam[] = []
          for (let i = 0; i < this.columns.length; i = i + 1) {
            params.push({
              value: resp.table.rows[rawIndex][i],
              col: this.columns[i]
            })
          }

          const irow: IRow = {
            params: params,
            rawIndex: rawIndex,
            cmdName: this.selectedCmd!.name
          }

          controls[rawIndex] = new RowCtrl(irow)
        }
      }
      this.rows$.next(controls)
    },
      err => console.error('execCmd error: ' + err),
      () => 
         { 
		 console.log('execCmd completed: ');
         if (control.isResUpdatable() ) {       
            if (!(control.ResUpdTimerSubscriber) || control.ResUpdTimerSubscriber.closed) {
			  if ( !control.ResUpdTimer)
			    control.ResUpdTimer = timer(1000, 1000);
              control.ResUpdTimerSubscriber = control.ResUpdTimer.subscribe (
                iteration => {
                console.log("Update timer fired" + iteration);
                if (control.updbtnname === "Stop update")
                  this.execCmd$(control);
                }
              );
            }
          }
		},
    ) // map resp


    return of(null);
  }

  onParamSubmit(control: RowCtrl) {
    this.commandsApi.setCmdParams$(control.api(), this.selectedModule!.name).subscribe(() => this.execCmd$(new CmdCtrl(this.selectedCmd!)));
  }

}
