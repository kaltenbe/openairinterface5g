# * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
# * contributor license agreements.  See the NOTICE file distributed with
# * this work for additional information regarding copyright ownership.
# * The OpenAirInterface Software Alliance licenses this file to You under
# * the OAI Public License, Version 1.1  (the "License"); you may not use this file
# * except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *      http://www.openairinterface.org/?page_id=698
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *-------------------------------------------------------------------------------
# * For more information about the OpenAirInterface (OAI) Software Alliance:
# *      contact@openairinterface.org
# */
#---------------------------------------------------------------------
#
#   Required Python Version
#     Python 3.x
#
#---------------------------------------------------------------------

#to use isfile
import os
import sys
import logging
#to create a SSH object locally in the methods
import sshconnection
#time.sleep
import time


import re
import subprocess

from datetime import datetime


class AS_UE:

    def __init__(self,Module):
        #create attributes as in the UE dictionary
        for k, v in Module.items():
            setattr(self, k, v)
        self.cmd_to_ue = { "attach" : self.WakeupScript, "detach" : self.DetachScript}
        self.UEIPAddress = ''




#-----------------$
#PUBLIC Methods$
#-----------------$

    def WaitEndScenario(self):
        logging.debug('waiting for scenario duration')
        time.sleep(int(self.Duration))

    def KillASUE(self):
        mySSH = sshconnection.SSHConnection()
        mySSH.open(self.HostIPAddress, self.HostUsername, self.HostPassword)
        mySSH.command('killall --signal SIGKILL lteue-avx2', '#', 5)
        mySSH.close()

    def RunScenario(self):
        mySSH = sshconnection.SSHConnection()
        mySSH.open(self.HostIPAddress, self.HostUsername, self.HostPassword)

        logging.debug("Deleting old artifacts :")
        cmd='rm -rf ' + self.Ping + ' ' + self.UELog
        mySSH.command(cmd,'#',5)
        logging.debug("Running scenario :")
        cmd='echo $USER; nohup '+self.Cmd + ' ' + self.Config + ' &'
        mySSH.command(cmd,'#',5)

        mySSH.close()

    def ASUEScenario(self, as_ue_cmd):
        mySSH = sshconnection.SSHConnection()
        mySSH.open(self.HostIPAddress, self.HostUsername, self.HostPassword)
        cmd = 'echo $USER; nohup '+self.cmd_to_ue[as_ue_cmd]
        mySSH.command(cmd,'#',5)
        mySSH.close()

    def GetModuleIPAddress(self):
        HOST=self.HostUsername+'@'+self.HostIPAddress
        response= []
        tentative = 8
        while (len(response)==0) and (tentative>0):
            COMMAND="ip netns exec ue1 ip a " + self.UENetwork + " | grep --colour=never inet | grep " + self.UENetwork
            if tentative == 8:
                logging.debug(COMMAND)
                ssh = subprocess.Popen(["ssh", "%s" % HOST, COMMAND],shell=False,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
                response = ssh.stdout.readlines()
                tentative-=1
                time.sleep(2)
                if (tentative==0) and (len(response)==0):
                    logging.debug('\u001B[1;37;41m Module IP Address Not Found! Time expired \u001B[0m')
                    return -1
                else: #check response
                    result = re.search('inet (?P<moduleipaddress>[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)', response[0].decode("utf-8") )
                    if result is not None:
                        if result.group('moduleipaddress') is not None:
                            self.UEIPAddress = result.group('moduleipaddress')
                            logging.debug('\u001B[1mUE Module IP Address is ' + self.UEIPAddress + '\u001B[0m')
                            return 0
                        else:
                            logging.debug('\u001B[1;37;41m Module IP Address Not Found! \u001B[0m')
                            return -1
                    else:
                        logging.debug('\u001B[1;37;41m Module IP Address Not Found! \u001B[0m')
                        return -1


