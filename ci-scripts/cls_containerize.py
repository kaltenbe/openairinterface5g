#/*
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
# Python for CI of OAI-eNB + COTS-UE
#
#   Required Python Version
#     Python 3.x
#
#   Required Python Package
#     pexpect
#---------------------------------------------------------------------

#-----------------------------------------------------------
# Import
#-----------------------------------------------------------
import sys              # arg
import re               # reg
import logging
import os
import shutil
import subprocess
import time
from multiprocessing import Process, Lock, SimpleQueue
from zipfile import ZipFile

#-----------------------------------------------------------
# OAI Testing modules
#-----------------------------------------------------------
import sshconnection as SSH
import helpreadme as HELP
import constants as CONST

#-----------------------------------------------------------
# Helper functions used here and in other classes
# (e.g., cls_cluster.py)
#-----------------------------------------------------------
def CreateWorkspace(sshSession, sourcePath, ranRepository, ranCommitID, ranTargetBranch, ranAllowMerge):
	if ranCommitID == '':
		logging.error('need ranCommitID in CreateWorkspace()')
		sys.exit('Insufficient Parameter in CreateWorkspace()')

	sshSession.command(f'rm -rf {sourcePath}', '\$', 10)
	sshSession.command('mkdir -p ' + sourcePath, '\$', 5)
	sshSession.command('cd ' + sourcePath, '\$', 5)
	# Recent version of git (>2.20?) should handle missing .git extension # without problems
	sshSession.command(f'git clone --filter=blob:none -n -b develop {ranRepository} .', '\$', 60)
	if sshSession.getBefore().count('error') > 0 or sshSession.getBefore().count('error') > 0:
		sys.exit('error during clone')
	sshSession.command('git config user.email "jenkins@openairinterface.org"', '\$', 5)
	sshSession.command('git config user.name "OAI Jenkins"', '\$', 5)

	sshSession.command('mkdir -p cmake_targets/log', '\$', 5)
	# if the commit ID is provided use it to point to it
	sshSession.command(f'git checkout -f {ranCommitID}', '\$', 30)
	if sshSession.getBefore().count(f'HEAD is now at {ranCommitID[:6]}') != 1:
		sshSession.command('git log --oneline | head -n5', '\$', 5)
		logging.warning(f'problems during checkout, is at: {sshSession.getBefore()}')
	else:
		logging.debug('successful checkout')
	# if the branch is not develop, then it is a merge request and we need to do
	# the potential merge. Note that merge conflicts should already been checked earlier
	if ranAllowMerge:
		if ranTargetBranch == '':
			ranTargetBranch = 'develop'
		logging.debug(f'Merging with the target branch: {ranTargetBranch}')
		sshSession.command(f'git merge --ff origin/{ranTargetBranch} -m "Temporary merge for CI"', '\$', 5)

def CopyLogsToExecutor(sshSession, sourcePath, log_name, scpIp, scpUser, scpPw):
	sshSession.command(f'cd {sourcePath}/cmake_targets', '\$', 5)
	sshSession.command(f'rm -f {log_name}.zip', '\$', 5)
	sshSession.command(f'mkdir -p {log_name}', '\$', 5)
	sshSession.command(f'mv log/* {log_name}', '\$', 5)
	sshSession.command(f'zip -r -qq {log_name}.zip {log_name}', '\$', 5)

	# copy zip to executor for analysis
	if (os.path.isfile(f'./{log_name}.zip')):
		os.remove(f'./{log_name}.zip')
	if (os.path.isdir(f'./{log_name}')):
		shutil.rmtree(f'./{log_name}')
	sshSession.copyin(scpIp, scpUser, scpPw, f'{sourcePath}/cmake_targets/{log_name}.zip', '.')
	sshSession.command(f'rm -f {log_name}.zip','\$', 5)
	ZipFile(f'{log_name}.zip').extractall('.')

def AnalyzeBuildLogs(buildRoot, images, globalStatus):
	collectInfo = {}
	for image in images:
		files = {}
		file_list = [f for f in os.listdir(f'{buildRoot}/{image}') if os.path.isfile(os.path.join(f'{buildRoot}/{image}', f)) and f.endswith('.txt')]
		# Analyze the "sub-logs" of every target image
		for fil in file_list:
			errorandwarnings = {}
			warningsNo = 0
			errorsNo = 0
			with open(f'{buildRoot}/{image}/{fil}', mode='r') as inputfile:
				for line in inputfile:
					result = re.search(' ERROR ', str(line))
					if result is not None:
						errorsNo += 1
					result = re.search(' error:', str(line))
					if result is not None:
						errorsNo += 1
					result = re.search(' WARNING ', str(line))
					if result is not None:
						warningsNo += 1
					result = re.search(' warning:', str(line))
					if result is not None:
						warningsNo += 1
				errorandwarnings['errors'] = errorsNo
				errorandwarnings['warnings'] = warningsNo
				errorandwarnings['status'] = globalStatus
			files[fil] = errorandwarnings
		# Analyze the target image
		if os.path.isfile(f'{buildRoot}/{image}.log'):
			errorandwarnings = {}
			committed = False
			tagged = False
			with open(f'{buildRoot}/{image}.log', mode='r') as inputfile:
				startOfTargetImageCreation = False # check for tagged/committed only after image created
				for line in inputfile:
					result = re.search(f'FROM .* [aA][sS] {image}$', str(line))
					if result is not None:
						startOfTargetImageCreation = True
					if startOfTargetImageCreation:
						lineHasTag = re.search(f'Successfully tagged {image}:', str(line)) is not None
						tagged = tagged or lineHasTag
						# the OpenShift Cluster builder prepends image registry URL
						lineHasCommit = re.search(f'COMMIT [a-zA-Z0-9\.:/\-]*{image}', str(line)) is not None
						committed = committed or lineHasCommit
			errorandwarnings['errors'] = 0 if committed or tagged else 1
			errorandwarnings['warnings'] = 0
			errorandwarnings['status'] = committed or tagged
			files['Target Image Creation'] = errorandwarnings
		collectInfo[image] = files
	return collectInfo

#-----------------------------------------------------------
# Class Declaration
#-----------------------------------------------------------
class Containerize():

	def __init__(self):
		
		self.ranRepository = ''
		self.ranBranch = ''
		self.ranAllowMerge = False
		self.ranCommitID = ''
		self.ranTargetBranch = ''
		self.eNBIPAddress = ''
		self.eNBUserName = ''
		self.eNBPassword = ''
		self.eNBSourceCodePath = ''
		self.eNB1IPAddress = ''
		self.eNB1UserName = ''
		self.eNB1Password = ''
		self.eNB1SourceCodePath = ''
		self.eNB2IPAddress = ''
		self.eNB2UserName = ''
		self.eNB2Password = ''
		self.eNB2SourceCodePath = ''
		self.forcedWorkspaceCleanup = False
		self.imageKind = ''
		self.proxyCommit = None
		self.eNB_instance = 0
		self.eNB_serverId = ['', '', '']
		self.yamlPath = ['', '', '']
		self.services = ['', '', '']
		self.nb_healthy = [0, 0, 0]
		self.exitStatus = 0
		self.eNB_logFile = ['', '', '']

		self.testCase_id = ''

		self.flexranCtrlDeployed = False
		self.flexranCtrlIpAddress = ''
		self.cli = ''
		self.cliBuildOptions = ''
		self.dockerfileprefix = ''
		self.host = ''

		self.deployedContainers = []
		self.tsharkStarted = False
		self.pingContName = ''
		self.pingOptions = ''
		self.pingLossThreshold = ''
		self.svrContName = ''
		self.svrOptions = ''
		self.cliContName = ''
		self.cliOptions = ''

		self.imageToCopy = ''
		self.registrySvrId = ''
		self.testSvrId = ''

		#checkers from xml
		self.ran_checkers={}

#-----------------------------------------------------------
# Container management functions
#-----------------------------------------------------------

	def BuildImage(self, HTML):
		if self.ranRepository == '' or self.ranBranch == '' or self.ranCommitID == '':
			HELP.GenericHelp(CONST.Version)
			sys.exit('Insufficient Parameter')
		if self.eNB_serverId[self.eNB_instance] == '0':
			lIpAddr = self.eNBIPAddress
			lUserName = self.eNBUserName
			lPassWord = self.eNBPassword
			lSourcePath = self.eNBSourceCodePath
		elif self.eNB_serverId[self.eNB_instance] == '1':
			lIpAddr = self.eNB1IPAddress
			lUserName = self.eNB1UserName
			lPassWord = self.eNB1Password
			lSourcePath = self.eNB1SourceCodePath
		elif self.eNB_serverId[self.eNB_instance] == '2':
			lIpAddr = self.eNB2IPAddress
			lUserName = self.eNB2UserName
			lPassWord = self.eNB2Password
			lSourcePath = self.eNB2SourceCodePath
		if lIpAddr == '' or lUserName == '' or lPassWord == '' or lSourcePath == '':
			HELP.GenericHelp(CONST.Version)
			sys.exit('Insufficient Parameter')
		logging.debug('Building on server: ' + lIpAddr)
		mySSH = SSH.SSHConnection()
		mySSH.open(lIpAddr, lUserName, lPassWord)
	
		# Checking the hostname to get adapted on cli and dockerfileprefixes
		mySSH.command('hostnamectl', '\$', 5)
		result = re.search('Ubuntu|Red Hat',  mySSH.getBefore())
		self.host = result.group(0)
		if self.host == 'Ubuntu':
			self.cli = 'docker'
			self.dockerfileprefix = '.ubuntu18'
			self.cliBuildOptions = '--no-cache'
		elif self.host == 'Red Hat':
			self.cli = 'sudo podman'
			self.dockerfileprefix = '.rhel8.2'
			self.cliBuildOptions = '--no-cache --disable-compression'

                # we always build the ran-build image with all targets
		imageNames = [('ran-build', 'build')]
		result = re.search('eNB', self.imageKind)
		# Creating a tupple with the imageName and the DockerFile prefix pattern on obelix
		if result is not None:
			imageNames.append(('oai-enb', 'eNB'))
		else:
			result = re.search('gNB', self.imageKind)
			if result is not None:
				imageNames.append(('oai-gnb', 'gNB'))
			else:
				result = re.search('all', self.imageKind)
				if result is not None:
					imageNames.append(('oai-enb', 'eNB'))
					imageNames.append(('oai-gnb', 'gNB'))
					imageNames.append(('oai-lte-ue', 'lteUE'))
					imageNames.append(('oai-nr-ue', 'nrUE'))
					if self.host == 'Red Hat':
						imageNames.append(('oai-physim', 'phySim'))
					if self.host == 'Ubuntu':
						imageNames.append(('oai-lte-ru', 'lteRU'))
		
		# Workaround for some servers, we need to erase completely the workspace
		if self.forcedWorkspaceCleanup:
			mySSH.command('echo ' + lPassWord + ' | sudo -S rm -Rf ' + lSourcePath, '\$', 15)
	
		self.testCase_id = HTML.testCase_id
	
		CreateWorkspace(mySSH, lSourcePath, self.ranRepository, self.ranCommitID, self.ranTargetBranch, self.ranAllowMerge)

 		# if asterix, copy the entitlement and subscription manager configurations
		if self.host == 'Red Hat':
			mySSH.command('mkdir -p ./etc-pki-entitlement ./rhsm-conf ./rhsm-ca', '\$', 5)
			mySSH.command('sudo cp /etc/rhsm/rhsm.conf ./rhsm-conf/', '\$', 5)
			mySSH.command('sudo cp /etc/rhsm/ca/redhat-uep.pem ./rhsm-ca/', '\$', 5)
			mySSH.command('sudo cp /etc/pki/entitlement/*.pem ./etc-pki-entitlement/', '\$', 5)

		baseImage = 'ran-base'
		baseTag = 'develop'
		forceBaseImageBuild = False
		imageTag = 'develop'
		if (self.ranAllowMerge):
			imageTag = 'ci-temp'
			if self.ranTargetBranch == 'develop':
				mySSH.command('git diff HEAD..origin/develop -- cmake_targets/build_oai cmake_targets/tools/build_helper docker/Dockerfile.base' + self.dockerfileprefix + ' | grep --colour=never -i INDEX', '\$', 5)
				result = re.search('index', mySSH.getBefore())
				if result is not None:
					forceBaseImageBuild = True
					baseTag = 'ci-temp'
		else:
			forceBaseImageBuild = True

		# Let's remove any previous run artifacts if still there
		mySSH.command(self.cli + ' image prune --force', '\$', 30)
		if forceBaseImageBuild:
			mySSH.command(self.cli + ' image rm ' + baseImage + ':' + baseTag + ' || true', '\$', 30)
		for image,pattern in imageNames:
			mySSH.command(self.cli + ' image rm ' + image + ':' + imageTag + ' || true', '\$', 30)

		# Build the base image only on Push Events (not on Merge Requests)
		# On when the base image docker file is being modified.
		if forceBaseImageBuild:
			mySSH.command(self.cli + ' build ' + self.cliBuildOptions + ' --target ' + baseImage + ' --tag ' + baseImage + ':' + baseTag + ' --file docker/Dockerfile.base' + self.dockerfileprefix + ' . > cmake_targets/log/ran-base.log 2>&1', '\$', 1600)
		# First verify if the base image was properly created.
		mySSH.command(self.cli + ' image inspect --format=\'Size = {{.Size}} bytes\' ' + baseImage + ':' + baseTag, '\$', 5)
		allImagesSize = {}
		if mySSH.getBefore().count('o such image') != 0:
			logging.error('\u001B[1m Could not build properly ran-base\u001B[0m')
			# Recover the name of the failed container?
			mySSH.command(self.cli + ' ps --quiet --filter "status=exited" -n1 | xargs ' + self.cli + ' rm -f', '\$', 5)
			mySSH.command(self.cli + ' image prune --force', '\$', 30)
			mySSH.close()
			logging.error('\u001B[1m Building OAI Images Failed\u001B[0m')
			HTML.CreateHtmlTestRow(self.imageKind, 'KO', CONST.ALL_PROCESSES_OK)
			HTML.CreateHtmlTabFooter(False)
			sys.exit(1)
		else:
			result = re.search('Size *= *(?P<size>[0-9\-]+) *bytes', mySSH.getBefore())
			if result is not None:
				size = float(result.group("size")) / 1000000
				imageSizeStr = f'{size:.1f}'
				logging.debug(f'\u001B[1m   ran-base size is {imageSizeStr} Mbytes\u001B[0m')
				allImagesSize['ran-base'] = f'{imageSizeStr} Mbytes'
			else:
				logging.debug('ran-base size is unknown')

		# Recover build logs, for the moment only possible when build is successful
		mySSH.command(self.cli + ' create --name test ' + baseImage + ':' + baseTag, '\$', 5)
		mySSH.command('mkdir -p cmake_targets/log/ran-base', '\$', 5)
		mySSH.command(self.cli + ' cp test:/oai-ran/cmake_targets/log/. cmake_targets/log/ran-base', '\$', 5)
		mySSH.command(self.cli + ' rm -f test', '\$', 5)

		# Build the target image(s)
		status = True
		attemptedImages = ['ran-base']
		for image,pattern in imageNames:
			attemptedImages += [image]
			# the archived Dockerfiles have "ran-base:latest" as base image
			# we need to update them with proper tag
			mySSH.command(f'sed -i -e "s#{baseImage}:latest#{baseImage}:{baseTag}#" docker/Dockerfile.{pattern}{self.dockerfileprefix}', '\$', 5)
			if image != 'ran-build':
				mySSH.command(f'sed -i -e "s#ran-build:latest#ran-build:{imageTag}#" docker/Dockerfile.{pattern}{self.dockerfileprefix}', '\$', 5)
			mySSH.command(f'{self.cli} build {self.cliBuildOptions} --target {image} --tag {image}:{imageTag} --file docker/Dockerfile.{pattern}{self.dockerfileprefix} . > cmake_targets/log/{image}.log 2>&1', '\$', 1200)
			# split the log
			mySSH.command('mkdir -p cmake_targets/log/' + image, '\$', 5)
			mySSH.command('python3 ci-scripts/docker_log_split.py --logfilename=cmake_targets/log/' + image + '.log', '\$', 5)
			# checking the status of the build
			mySSH.command(self.cli + ' image inspect --format=\'Size = {{.Size}} bytes\' ' + image + ':' + imageTag, '\$', 5)
			if mySSH.getBefore().count('o such image') != 0:
				logging.error('\u001B[1m Could not build properly ' + image + '\u001B[0m')
				status = False
				# Here we should check if the last container corresponds to a failed command and destroy it
				mySSH.command(self.cli + ' ps --quiet --filter "status=exited" -n1 | xargs ' + self.cli + ' rm -f', '\$', 5)
				allImagesSize[image] = 'N/A -- Build Failed'
				break
			else:
				result = re.search('Size *= *(?P<size>[0-9\-]+) *bytes', mySSH.getBefore())
				if result is not None:
					size = float(result.group("size")) / 1000000
					imageSizeStr = f'{size:.1f}'
					logging.debug(f'\u001B[1m   {image} size is {imageSizeStr} Mbytes\u001B[0m')
					allImagesSize[image] = f'{imageSizeStr} Mbytes'
				else:
					logging.debug(f'{image} size is unknown')
					allImagesSize[image] = 'unknown'
			# Now pruning dangling images in between target builds
			mySSH.command(self.cli + ' image prune --force', '\$', 30)

		# Remove all intermediate build images and clean up
		if self.ranAllowMerge and forceBaseImageBuild:
			mySSH.command(f'{self.cli} image rm {baseImage}:{baseTag} || true', '\$', 30)
		mySSH.command(f'{self.cli} image rm ran-build:{imageTag}','\$', 30)
		mySSH.command(f'{self.cli} volume prune --force','\$', 15)

		# create a zip with all logs
		build_log_name = f'build_log_{self.testCase_id}'
		CopyLogsToExecutor(mySSH, lSourcePath, build_log_name, lIpAddr, lUserName, lPassWord)
		mySSH.close()

		# Analyze the logs
		collectInfo = AnalyzeBuildLogs(build_log_name, attemptedImages, status)
		
		if status:
			logging.info('\u001B[1m Building OAI Image(s) Pass\u001B[0m')
			HTML.CreateHtmlTestRow(self.imageKind, 'OK', CONST.ALL_PROCESSES_OK)
			HTML.CreateHtmlNextTabHeaderTestRow(collectInfo, allImagesSize)
		else:
			logging.error('\u001B[1m Building OAI Images Failed\u001B[0m')
			HTML.CreateHtmlTestRow(self.imageKind, 'KO', CONST.ALL_PROCESSES_OK)
			HTML.CreateHtmlNextTabHeaderTestRow(collectInfo, allImagesSize)
			HTML.CreateHtmlTabFooter(False)
			sys.exit(1)

	def BuildProxy(self, HTML):
		if self.ranRepository == '' or self.ranBranch == '' or self.ranCommitID == '':
			HELP.GenericHelp(CONST.Version)
			sys.exit('Insufficient Parameter')
		if self.eNB_serverId[self.eNB_instance] == '0':
			lIpAddr = self.eNBIPAddress
			lUserName = self.eNBUserName
			lPassWord = self.eNBPassword
			lSourcePath = self.eNBSourceCodePath
		elif self.eNB_serverId[self.eNB_instance] == '1':
			lIpAddr = self.eNB1IPAddress
			lUserName = self.eNB1UserName
			lPassWord = self.eNB1Password
			lSourcePath = self.eNB1SourceCodePath
		elif self.eNB_serverId[self.eNB_instance] == '2':
			lIpAddr = self.eNB2IPAddress
			lUserName = self.eNB2UserName
			lPassWord = self.eNB2Password
			lSourcePath = self.eNB2SourceCodePath
		if lIpAddr == '' or lUserName == '' or lPassWord == '' or lSourcePath == '':
			HELP.GenericHelp(CONST.Version)
			sys.exit('Insufficient Parameter')
		if self.proxyCommit is None:
			HELP.GenericHelp(CONST.Version)
			sys.exit('Insufficient Parameter (need proxyCommit for proxy build)')
		logging.debug('Building on server: ' + lIpAddr)
		mySSH = SSH.SSHConnection()
		mySSH.open(lIpAddr, lUserName, lPassWord)

		# Check that we are on Ubuntu
		mySSH.command('hostnamectl', '\$', 5)
		result = re.search('Ubuntu',  mySSH.getBefore())
		self.host = result.group(0)
		if self.host != 'Ubuntu':
			logging.error('\u001B[1m Can build proxy only on Ubuntu server\u001B[0m')
			mySSH.close()
			sys.exit(1)

		self.cli = 'docker'
		self.cliBuildOptions = '--no-cache'

		# Workaround for some servers, we need to erase completely the workspace
		if self.forcedWorkspaceCleanup:
			mySSH.command('echo ' + lPassWord + ' | sudo -S rm -Rf ' + lSourcePath, '\$', 15)

		oldRanCommidID = self.ranCommitID
		oldRanRepository = self.ranRepository
		oldRanAllowMerge = self.ranAllowMerge
		self.ranCommitID = self.proxyCommit
		self.ranRepository = 'https://github.com/EpiSci/oai-lte-5g-multi-ue-proxy.git'
		self.ranAllowMerge = False
		CreateWorkspace(mySSH, lSourcePath, self.ranRepository, self.ranCommitID, self.ranTargetBranch, self.ranAllowMerge)
		# to prevent accidentally overwriting data that might be used later
		self.ranCommitID = oldRanCommidID
		self.ranRepository = oldRanRepository
		self.ranAllowMerge = oldRanAllowMerge

		# Let's remove any previous run artifacts if still there
		mySSH.command(self.cli + ' image prune --force', '\$', 30)
		# Remove any previous proxy image
		mySSH.command(self.cli + ' image rm oai-lte-multi-ue-proxy:latest || true', '\$', 30)

		tag = self.proxyCommit
		logging.debug('building L2sim proxy image for tag ' + tag)
		# check if the corresponding proxy image with tag exists. If not, build it
		mySSH.command(self.cli + ' image inspect --format=\'Size = {{.Size}} bytes\' proxy:' + tag, '\$', 5)
		buildProxy = mySSH.getBefore().count('o such image') != 0
		if buildProxy:
			mySSH.command(self.cli + ' build ' + self.cliBuildOptions + ' --target oai-lte-multi-ue-proxy --tag proxy:' + tag + ' --file docker/Dockerfile.ubuntu18.04 . > cmake_targets/log/proxy-build.log 2>&1', '\$', 180)
			# Note: at this point, OAI images are flattened, but we cannot do this
			# here, as the flatten script is not in the proxy repo
			mySSH.command(self.cli + ' image inspect --format=\'Size = {{.Size}} bytes\' proxy:' + tag, '\$', 5)
			if mySSH.getBefore().count('o such image') != 0:
				logging.error('\u001B[1m Build of L2sim proxy failed\u001B[0m')
				mySSH.close()
				HTML.CreateHtmlTestRow('commit ' + tag, 'KO', CONST.ALL_PROCESSES_OK)
				HTML.CreateHtmlTabFooter(False)
				sys.exit(1)
		else:
			logging.debug('L2sim proxy image for tag ' + tag + ' already exists, skipping build')

		# retag the build images to that we pick it up later
		mySSH.command('docker image tag proxy:' + tag + ' oai-lte-multi-ue-proxy:latest', '\$', 5)

		# no merge: is a push to develop, tag the image so we can push it to the registry
		if not self.ranAllowMerge:
			mySSH.command('docker image tag proxy:' + tag + ' proxy:develop', '\$', 5)

		# we assume that the host on which this is built will also run the proxy. The proxy
		# currently requires the following command, and the docker-compose up mechanism of
		# the CI does not allow to run arbitrary commands. Note that the following actually
		# belongs to the deployment, not the build of the proxy...
		logging.warning('the following command belongs to deployment, but no mechanism exists to exec it there!')
		mySSH.command('sudo ifconfig lo: 127.0.0.2 netmask 255.0.0.0 up', '\$', 5)

		# Analyzing the logs
		if buildProxy:
			self.testCase_id = HTML.testCase_id
			mySSH.command('cd ' + lSourcePath + '/cmake_targets', '\$', 5)
			mySSH.command('mkdir -p proxy_build_log_' + self.testCase_id, '\$', 5)
			mySSH.command('mv log/* ' + 'proxy_build_log_' + self.testCase_id, '\$', 5)
			if (os.path.isfile('./proxy_build_log_' + self.testCase_id + '.zip')):
				os.remove('./proxy_build_log_' + self.testCase_id + '.zip')
			if (os.path.isdir('./proxy_build_log_' + self.testCase_id)):
				shutil.rmtree('./proxy_build_log_' + self.testCase_id)
			mySSH.command('zip -r -qq proxy_build_log_' + self.testCase_id + '.zip proxy_build_log_' + self.testCase_id, '\$', 5)
			mySSH.copyin(lIpAddr, lUserName, lPassWord, lSourcePath + '/cmake_targets/build_log_' + self.testCase_id + '.zip', '.')
			# don't delete such that we might recover the zips
			#mySSH.command('rm -f build_log_' + self.testCase_id + '.zip','\$', 5)

		# we do not analyze the logs (we assume the proxy builds fine at this stage),
		# but need to have the following information to correctly display the HTML
		files = {}
		errorandwarnings = {}
		errorandwarnings['errors'] = 0
		errorandwarnings['warnings'] = 0
		errorandwarnings['status'] = True
		files['Target Image Creation'] = errorandwarnings
		collectInfo = {}
		collectInfo['proxy'] = files
		mySSH.command('docker image inspect --format=\'Size = {{.Size}} bytes\' proxy:' + tag, '\$', 5)
		result = re.search('Size *= *(?P<size>[0-9\-]+) *bytes', mySSH.getBefore())
		allImagesSize = {}
		if result is not None:
			imageSize = float(result.group('size')) / 1000000
			logging.debug('\u001B[1m   proxy size is ' + ('%.0f' % imageSize) + ' Mbytes\u001B[0m')
			allImagesSize['proxy'] = str(round(imageSize,1)) + ' Mbytes'
		else:
			logging.debug('proxy size is unknown')
			allImagesSize['proxy'] = 'unknown'

		# Cleaning any created tmp volume
		mySSH.command(self.cli + ' volume prune --force || true','\$', 15)
		mySSH.close()

		logging.info('\u001B[1m Building L2sim Proxy Image Pass\u001B[0m')
		HTML.CreateHtmlTestRow('commit ' + tag, 'OK', CONST.ALL_PROCESSES_OK)
		HTML.CreateHtmlNextTabHeaderTestRow(collectInfo, allImagesSize)

	def Copy_Image_to_Test_Server(self, HTML):
		imageTag = 'develop'
		if (self.ranAllowMerge):
			imageTag = 'ci-temp'

		lSsh = SSH.SSHConnection()
		# Going to the Docker Registry server
		if self.registrySvrId == '0':
			lIpAddr = self.eNBIPAddress
			lUserName = self.eNBUserName
			lPassWord = self.eNBPassword
		elif self.registrySvrId == '1':
			lIpAddr = self.eNB1IPAddress
			lUserName = self.eNB1UserName
			lPassWord = self.eNB1Password
		elif self.registrySvrId == '2':
			lIpAddr = self.eNB2IPAddress
			lUserName = self.eNB2UserName
			lPassWord = self.eNB2Password
		lSsh.open(lIpAddr, lUserName, lPassWord)
		lSsh.command('docker save ' + self.imageToCopy + ':' + imageTag + ' | gzip --fast > ' + self.imageToCopy + '-' + imageTag + '.tar.gz', '\$', 60)
		ret = lSsh.copyin(lIpAddr, lUserName, lPassWord, '~/' + self.imageToCopy + '-' + imageTag + '.tar.gz', '.')
		if ret != 0:
			HTML.CreateHtmlTestRow('N/A', 'KO', CONST.ALL_PROCESSES_OK)
			self.exitStatus = 1
			return False
		lSsh.command('rm ' + self.imageToCopy + '-' + imageTag + '.tar.gz', '\$', 60)
		if lSsh.getBefore().count('cannot remove'):
			HTML.CreateHtmlTestRow('file not created by docker save', 'KO', CONST.ALL_PROCESSES_OK)
			self.exitStatus = 1
			return False
		lSsh.close()

		# Going to the Test Server
		if self.testSvrId == '0':
			lIpAddr = self.eNBIPAddress
			lUserName = self.eNBUserName
			lPassWord = self.eNBPassword
		elif self.testSvrId == '1':
			lIpAddr = self.eNB1IPAddress
			lUserName = self.eNB1UserName
			lPassWord = self.eNB1Password
		elif self.testSvrId == '2':
			lIpAddr = self.eNB2IPAddress
			lUserName = self.eNB2UserName
			lPassWord = self.eNB2Password
		lSsh.open(lIpAddr, lUserName, lPassWord)
		lSsh.copyout(lIpAddr, lUserName, lPassWord, './' + self.imageToCopy + '-' + imageTag + '.tar.gz', '~')
		# copyout has no return code and will quit if something fails
		lSsh.command('docker rmi ' + self.imageToCopy + ':' + imageTag, '\$', 10)
		lSsh.command('docker load < ' + self.imageToCopy + '-' + imageTag + '.tar.gz', '\$', 60)
		if lSsh.getBefore().count('o such file') or lSsh.getBefore().count('invalid tar header'):
			logging.debug(lSsh.getBefore())
			HTML.CreateHtmlTestRow('problem during docker load', 'KO', CONST.ALL_PROCESSES_OK)
			self.exitStatus = 1
			return False
		lSsh.command('rm ' + self.imageToCopy + '-' + imageTag + '.tar.gz', '\$', 60)
		if lSsh.getBefore().count('cannot remove'):
			HTML.CreateHtmlTestRow('file not copied during scp?', 'KO', CONST.ALL_PROCESSES_OK)
			self.exitStatus = 1
			return False
		lSsh.close()

		if os.path.isfile('./' + self.imageToCopy + '-' + imageTag + '.tar.gz'):
			os.remove('./' + self.imageToCopy + '-' + imageTag + '.tar.gz')

		HTML.CreateHtmlTestRow('N/A', 'OK', CONST.ALL_PROCESSES_OK)
		return True

	def DeployObject(self, HTML, EPC):
		if self.eNB_serverId[self.eNB_instance] == '0':
			lIpAddr = self.eNBIPAddress
			lUserName = self.eNBUserName
			lPassWord = self.eNBPassword
			lSourcePath = self.eNBSourceCodePath
		elif self.eNB_serverId[self.eNB_instance] == '1':
			lIpAddr = self.eNB1IPAddress
			lUserName = self.eNB1UserName
			lPassWord = self.eNB1Password
			lSourcePath = self.eNB1SourceCodePath
		elif self.eNB_serverId[self.eNB_instance] == '2':
			lIpAddr = self.eNB2IPAddress
			lUserName = self.eNB2UserName
			lPassWord = self.eNB2Password
			lSourcePath = self.eNB2SourceCodePath
		if lIpAddr == '' or lUserName == '' or lPassWord == '' or lSourcePath == '':
			HELP.GenericHelp(CONST.Version)
			sys.exit('Insufficient Parameter')
		logging.debug('\u001B[1m Deploying OAI Object on server: ' + lIpAddr + '\u001B[0m')

		mySSH = SSH.SSHConnection()
		mySSH.open(lIpAddr, lUserName, lPassWord)
		
		CreateWorkspace(mySSH, lSourcePath, self.ranRepository, self.ranCommitID, self.ranTargetBranch, self.ranAllowMerge)

		mySSH.command('cd ' + lSourcePath + '/' + self.yamlPath[self.eNB_instance], '\$', 5)
		mySSH.command('cp docker-compose.yml ci-docker-compose.yml', '\$', 5)
		imageTag = 'develop'
		if (self.ranAllowMerge):
			imageTag = 'ci-temp'
		mySSH.command('sed -i -e "s/image: oai-enb:latest/image: oai-enb:' + imageTag + '/" ci-docker-compose.yml', '\$', 2)
		mySSH.command('sed -i -e "s/image: oai-gnb:latest/image: oai-gnb:' + imageTag + '/" ci-docker-compose.yml', '\$', 2)
		localMmeIpAddr = EPC.MmeIPAddress
		mySSH.command('sed -i -e "s/CI_MME_IP_ADDR/' + localMmeIpAddr + '/" ci-docker-compose.yml', '\$', 2)
#		if self.flexranCtrlDeployed:
#			mySSH.command('sed -i -e "s/FLEXRAN_ENABLED:.*/FLEXRAN_ENABLED: \'yes\'/" ci-docker-compose.yml', '\$', 2)
#			mySSH.command('sed -i -e "s/CI_FLEXRAN_CTL_IP_ADDR/' + self.flexranCtrlIpAddress + '/" ci-docker-compose.yml', '\$', 2)
#		else:
#			mySSH.command('sed -i -e "s/FLEXRAN_ENABLED:.*$/FLEXRAN_ENABLED: \'no\'/" ci-docker-compose.yml', '\$', 2)
#			mySSH.command('sed -i -e "s/CI_FLEXRAN_CTL_IP_ADDR/127.0.0.1/" ci-docker-compose.yml', '\$', 2)
		# Currently support only one
		mySSH.command('echo ' + lPassWord + ' | sudo -S b2xx_fx3_utils --reset-device', '\$', 15)
		mySSH.command('docker-compose --file ci-docker-compose.yml config --services | sed -e "s@^@service=@" 2>&1', '\$', 10)
		result = re.search('service=(?P<svc_name>[a-zA-Z0-9\_]+)', mySSH.getBefore())
		if result is not None:
			svcName = result.group('svc_name')
			mySSH.command('docker-compose --file ci-docker-compose.yml up -d ' + svcName, '\$', 15)

		# Checking Status
		mySSH.command('docker-compose --file ci-docker-compose.yml config', '\$', 5)
		result = re.search('container_name: (?P<container_name>[a-zA-Z0-9\-\_]+)', mySSH.getBefore())
		unhealthyNb = 0
		healthyNb = 0
		startingNb = 0
		containerName = ''
		if result is not None:
			containerName = result.group('container_name')
			time.sleep(5)
			cnt = 0
			while (cnt < 3):
				mySSH.command('docker inspect --format="{{.State.Health.Status}}" ' + containerName, '\$', 5)
				unhealthyNb = mySSH.getBefore().count('unhealthy')
				healthyNb = mySSH.getBefore().count('healthy') - unhealthyNb
				startingNb = mySSH.getBefore().count('starting')
				if healthyNb == 1:
					cnt = 10
				else:
					time.sleep(10)
					cnt += 1
		logging.debug(' -- ' + str(healthyNb) + ' healthy container(s)')
		logging.debug(' -- ' + str(unhealthyNb) + ' unhealthy container(s)')
		logging.debug(' -- ' + str(startingNb) + ' still starting container(s)')

		self.testCase_id = HTML.testCase_id
		self.eNB_logFile[self.eNB_instance] = 'enb_' + self.testCase_id + '.log'

		status = False
		if healthyNb == 1:
			cnt = 0
			while (cnt < 20):
				mySSH.command('docker logs ' + containerName + ' | egrep --text --color=never -i "wait|sync|Starting"', '\$', 30) 
				result = re.search('got sync|Starting F1AP at CU', mySSH.getBefore())
				if result is None:
					time.sleep(6)
					cnt += 1
				else:
					cnt = 100
					status = True
					logging.info('\u001B[1m Deploying OAI object Pass\u001B[0m')
					time.sleep(10)
		else:
			# containers are unhealthy, so we won't start. However, logs are stored at the end
			# in UndeployObject so we here store the logs of the unhealthy container to report it
			logfilename = f'{lSourcePath}/cmake_targets/{self.eNB_logFile[self.eNB_instance]}'
			mySSH.command('docker logs {containerName} > {logfilename}', '\$', 30)
			mySSH.copyin(lIpAddr, lUserName, lPassWord, logfilename, '.')
		mySSH.close()

		if status:
			HTML.CreateHtmlTestRow('N/A', 'OK', CONST.ALL_PROCESSES_OK)
		else:
			self.exitStatus = 1
			HTML.CreateHtmlTestRow('N/A', 'KO', CONST.ALL_PROCESSES_OK)


	def UndeployObject(self, HTML, RAN):
		if self.eNB_serverId[self.eNB_instance] == '0':
			lIpAddr = self.eNBIPAddress
			lUserName = self.eNBUserName
			lPassWord = self.eNBPassword
			lSourcePath = self.eNBSourceCodePath
		elif self.eNB_serverId[self.eNB_instance] == '1':
			lIpAddr = self.eNB1IPAddress
			lUserName = self.eNB1UserName
			lPassWord = self.eNB1Password
			lSourcePath = self.eNB1SourceCodePath
		elif self.eNB_serverId[self.eNB_instance] == '2':
			lIpAddr = self.eNB2IPAddress
			lUserName = self.eNB2UserName
			lPassWord = self.eNB2Password
			lSourcePath = self.eNB2SourceCodePath
		if lIpAddr == '' or lUserName == '' or lPassWord == '' or lSourcePath == '':
			HELP.GenericHelp(CONST.Version)
			sys.exit('Insufficient Parameter')
		logging.debug('\u001B[1m Undeploying OAI Object from server: ' + lIpAddr + '\u001B[0m')
		mySSH = SSH.SSHConnection()
		mySSH.open(lIpAddr, lUserName, lPassWord)
		mySSH.command('cd ' + lSourcePath + '/' + self.yamlPath[self.eNB_instance], '\$', 5)
		# Currently support only one
		mySSH.command('docker-compose --file ci-docker-compose.yml config', '\$', 5)
		containerName = ''
		containerToKill = False
		result = re.search('container_name: (?P<container_name>[a-zA-Z0-9\-\_]+)', mySSH.getBefore())
		if self.eNB_logFile[self.eNB_instance] == '':
			self.eNB_logFile[self.eNB_instance] = 'enb_' + HTML.testCase_id + '.log'
		if result is not None:
			containerName = result.group('container_name')
			containerToKill = True
		if containerToKill:
			mySSH.command('docker inspect ' + containerName, '\$', 30)
			result = re.search('Error: No such object: ' + containerName, mySSH.getBefore())
			if result is not None:
				containerToKill = False
		if containerToKill:
			mySSH.command('docker kill --signal INT ' + containerName, '\$', 30)
			time.sleep(5)
			mySSH.command('docker kill --signal KILL ' + containerName, '\$', 30)
			time.sleep(5)
			mySSH.command('docker logs ' + containerName + ' > ' + lSourcePath + '/cmake_targets/' + self.eNB_logFile[self.eNB_instance], '\$', 30)
			mySSH.command('docker rm -f ' + containerName, '\$', 30)
		# Forcing the down now to remove the networks and any artifacts
		mySSH.command('docker-compose --file ci-docker-compose.yml down', '\$', 5)
		# Cleaning any created tmp volume
		mySSH.command('docker volume prune --force || true', '\$', 20)

		mySSH.close()

		# Analyzing log file!
		if containerToKill:
			copyin_res = mySSH.copyin(lIpAddr, lUserName, lPassWord, lSourcePath + '/cmake_targets/' + self.eNB_logFile[self.eNB_instance], '.')
		else:
			copyin_res = 0
		nodeB_prefix = 'e'
		if (copyin_res == -1):
			HTML.htmleNBFailureMsg='Could not copy ' + nodeB_prefix + 'NB logfile to analyze it!'
			HTML.CreateHtmlTestRow('N/A', 'KO', CONST.ENB_PROCESS_NOLOGFILE_TO_ANALYZE)
		else:
			if containerToKill:
				logging.debug('\u001B[1m Analyzing ' + nodeB_prefix + 'NB logfile \u001B[0m ' + self.eNB_logFile[self.eNB_instance])
				logStatus = RAN.AnalyzeLogFile_eNB(self.eNB_logFile[self.eNB_instance], HTML, self.ran_checkers)
			else:
				logStatus = 0
			if (logStatus < 0):
				HTML.CreateHtmlTestRow(RAN.runtime_stats, 'KO', logStatus)
			else:
				HTML.CreateHtmlTestRow(RAN.runtime_stats, 'OK', CONST.ALL_PROCESSES_OK)
			# all the xNB run logs shall be on the server 0 for logCollecting
			if containerToKill and self.eNB_serverId[self.eNB_instance] != '0':
				mySSH.copyout(self.eNBIPAddress, self.eNBUserName, self.eNBPassword, './' + self.eNB_logFile[self.eNB_instance], self.eNBSourceCodePath + '/cmake_targets/')
		logging.info('\u001B[1m Undeploying OAI Object Pass\u001B[0m')

	def DeployGenObject(self, HTML, RAN, UE):
		self.exitStatus = 0
		logging.info('\u001B[1m Checking Services to deploy\u001B[0m')
		cmd = 'cd ' + self.yamlPath[0] + ' && docker-compose config --services'
		logging.debug(cmd)
		try:
			listServices = subprocess.check_output(cmd, shell=True, universal_newlines=True)
		except Exception as e:
			self.exitStatus = 1
			HTML.CreateHtmlTestRow('SVC not Found', 'KO', CONST.ALL_PROCESSES_OK)
			return
		for reqSvc in self.services[0].split(' '):
			res = re.search(reqSvc, listServices)
			if res is None:
				logging.error(reqSvc + ' not found in specified docker-compose')
				self.exitStatus = 1
		if (self.exitStatus == 1):
			HTML.CreateHtmlTestRow('SVC not Found', 'KO', CONST.ALL_PROCESSES_OK)
			return

		if (self.ranAllowMerge):
			cmd = 'cd ' + self.yamlPath[0] + ' && sed -e "s@develop@ci-temp@" docker-compose.y*ml > docker-compose-ci.yml'
		else:
			cmd = 'cd ' + self.yamlPath[0] + ' && sed -e "s@develop@develop@" docker-compose.y*ml > docker-compose-ci.yml'
		logging.debug(cmd)
		subprocess.run(cmd, shell=True)

		cmd = 'cd ' + self.yamlPath[0] + ' && docker-compose -f docker-compose-ci.yml up -d ' + self.services[0]
		logging.debug(cmd)
		try:
			deployStatus = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=100)
		except Exception as e:
			self.exitStatus = 1
			logging.error('Could not deploy')
			HTML.CreateHtmlTestRow('Could not deploy', 'KO', CONST.ALL_PROCESSES_OK)
			return

		logging.info('\u001B[1m Checking if all deployed healthy\u001B[0m')
		cmd = 'cd ' + self.yamlPath[0] + ' && docker-compose -f docker-compose-ci.yml ps -a'
		count = 0
		healthy = 0
		newContainers = []
		while (count < 10):
			count += 1
			containerStatus = []
			deployStatus = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=30)
			healthy = 0
			for state in deployStatus.split('\n'):
				res = re.search('Name|----------', state)
				if res is not None:
					continue
				if len(state) == 0:
					continue
				res = re.search('^(?P<container_name>[a-zA-Z0-9\-\_]+) ', state)
				if res is not None:
					cName = res.group('container_name')
					found = False
					for alreadyDeployed in self.deployedContainers:
						if cName == alreadyDeployed:
							found = True
					if not found:
						newContainers.append(cName)
						self.deployedContainers.append(cName)
				if re.search('Up \(healthy\)', state) is not None:
					healthy += 1
				if re.search('rfsim4g-db-init.*Exit 0', state) is not None:
					subprocess.check_output('docker rm -f rfsim4g-db-init || true', shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=30)
				containerStatus.append(state)
			if healthy == self.nb_healthy[0]:
				count = 100
			else:
				time.sleep(10)

		imagesInfo = ''
		for newCont in newContainers:
			cmd = 'docker inspect -f "{{.Config.Image}}" ' + newCont
			imageName = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=30)
			imageName = str(imageName).strip()
			cmd = 'docker image inspect --format "{{.RepoTags}}\t{{.Size}} bytes\t{{.Created}}\t{{.Id}}" ' + imageName
			imagesInfo += subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=30)

		html_queue = SimpleQueue()
		html_cell = '<pre style="background-color:white">\n'
		for imageInfo in imagesInfo.split('\n'):
			html_cell += imageInfo[:-11] + '\n'
		html_cell += '\n'
		for cState in containerStatus:
			html_cell += cState + '\n'
		html_cell += '</pre>'
		html_queue.put(html_cell)
		if count == 100 and healthy == self.nb_healthy[0]:
			if self.tsharkStarted == False:
				logging.debug('Starting tshark on public network')
				self.CaptureOnDockerNetworks()
			HTML.CreateHtmlTestRowQueue('n/a', 'OK', 1, html_queue)
			for cState in containerStatus:
				logging.debug(cState)
			logging.info('\u001B[1m Deploying OAI Object(s) PASS\u001B[0m')
		else:
			HTML.CreateHtmlTestRowQueue('Could not deploy in time', 'KO', 1, html_queue)
			for cState in containerStatus:
				logging.debug(cState)
			logging.error('\u001B[1m Deploying OAI Object(s) FAILED\u001B[0m')
			HTML.testCase_id = 'AUTO-UNDEPLOY'
			UE.testCase_id = 'AUTO-UNDEPLOY'
			HTML.desc = 'Automatic Undeployment'
			UE.desc = 'Automatic Undeployment'
			UE.ShowTestID()
			self.UndeployGenObject(HTML, RAN, UE)
			self.exitStatus = 1

	def CaptureOnDockerNetworks(self):
		cmd = 'cd ' + self.yamlPath[0] + ' && docker-compose -f docker-compose-ci.yml config | grep com.docker.network.bridge.name | sed -e "s@^.*name: @@"'
		networkNames = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=10)
		if re.search('4g.*rfsimulator', self.yamlPath[0]) is not None:
			# Excluding any traffic from LTE-UE container (192.168.61.30)
			# From the trf-gen, keeping only PING traffic
			cmd = 'sudo nohup tshark -f "(host 192.168.61.11 and icmp) or (not host 192.168.61.11 and not host 192.168.61.30 and not arp and not port 53 and not port 2152)"'
		elif re.search('5g.*rfsimulator', self.yamlPath[0]) is not None:
			# Excluding any traffic from NR-UE containers (192.168.71.150 and 192.168.71.151)
			# From the ext-dn, keeping only PING traffic
			cmd = 'sudo nohup tshark -f "(host 192.168.72.135 and icmp) or (not host 192.168.72.135 and not host 192.168.71.150 and not host 192.168.71.151 and not arp and not port 53 and not port 2152 and not port 2153)"'
		elif re.search('5g_l2sim', self.yamlPath[0]) is not None:
			cmd = 'sudo nohup tshark -f "(host 192.168.72.135 and icmp) or (not host 192.168.72.135 and not arp and not port 53 and not port 2152 and not port 2153)"'
		else:
			return
		for name in networkNames.split('\n'):
			if re.search('rfsim', name) is not None or re.search('l2sim', name) is not None:
				cmd += ' -i ' + name
		cmd += ' -w /tmp/capture_'
		ymlPath = self.yamlPath[0].split('/')
		cmd += ymlPath[1] + '.pcap > /tmp/tshark.log 2>&1 &'
		logging.debug(cmd)
		networkNames = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=10)
		self.tsharkStarted = True

	def UndeployGenObject(self, HTML, RAN, UE):
		self.exitStatus = 0
		ymlPath = self.yamlPath[0].split('/')
		logPath = '../cmake_targets/log/' + ymlPath[1]

		if (self.ranAllowMerge):
			cmd = 'cd ' + self.yamlPath[0] + ' && sed -e "s@develop@ci-temp@" docker-compose.y*ml > docker-compose-ci.yml'
		else:
			cmd = 'cd ' + self.yamlPath[0] + ' && sed -e "s@develop@develop@" docker-compose.y*ml > docker-compose-ci.yml'
		logging.debug(cmd)
		subprocess.run(cmd, shell=True)

		# check which containers are running for log recovery later
		cmd = 'cd ' + self.yamlPath[0] + ' && docker-compose -f docker-compose-ci.yml ps --all'
		logging.debug(cmd)
		deployStatusLogs = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=30)

		# Stop the containers to shut down objects
		logging.debug('\u001B[1m Stopping containers \u001B[0m')
		cmd = 'cd ' + self.yamlPath[0] + ' && docker-compose -f docker-compose-ci.yml stop'
		logging.debug(cmd)
		try:
			deployStatus = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=100)
		except Exception as e:
			self.exitStatus = 1
			logging.error('Could not stop containers')
			HTML.CreateHtmlTestRow('Could not stop', 'KO', CONST.ALL_PROCESSES_OK)
			logging.error('\u001B[1m Undeploying OAI Object(s) FAILED\u001B[0m')
			return

		anyLogs = False
		for state in deployStatusLogs.split('\n'):
			res = re.search('Name|----------', state)
			if res is not None:
				continue
			if len(state) == 0:
				continue
			res = re.search('^(?P<container_name>[a-zA-Z0-9\-\_]+) ', state)
			if res is not None:
				anyLogs = True
				cName = res.group('container_name')
				cmd = 'cd ' + self.yamlPath[0] + ' && docker logs ' + cName + ' > ' + cName + '.log 2>&1'
				logging.debug(cmd)
				subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=30)
				if re.search('magma-mme', cName) is not None:
					cmd = 'cd ' + self.yamlPath[0] + ' && docker cp -L ' + cName + ':/var/log/mme.log ' + cName + '-full.log'
					logging.debug(cmd)
					subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=30)
		fullStatus = True
		if anyLogs:
			cmd = 'mkdir -p '+ logPath + ' && cp ' + self.yamlPath[0] + '/*.log ' + logPath
			logging.debug(cmd)
			deployStatus = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=10)

			# Analyzing log file(s)!
			listOfPossibleRanContainers = ['enb', 'gnb', 'cu', 'du']
			for container in listOfPossibleRanContainers:
				filenames = self.yamlPath[0] + '/*-oai-' + container + '.log'
				cmd = 'ls ' + filenames
				containerStatus = True
				try:
					lsStatus = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=10)
					filenames = str(lsStatus).strip()
				except:
					containerStatus = False
				if not containerStatus:
					continue

				for filename in filenames.split('\n'):
					logging.debug('\u001B[1m Analyzing xNB logfile ' + filename + ' \u001B[0m')
					logStatus = RAN.AnalyzeLogFile_eNB(filename, HTML, self.ran_checkers)
					if (logStatus < 0):
						fullStatus = False
						self.exitStatus = 1
						HTML.CreateHtmlTestRow(RAN.runtime_stats, 'KO', logStatus)
					else:
						HTML.CreateHtmlTestRow(RAN.runtime_stats, 'OK', CONST.ALL_PROCESSES_OK)

			listOfPossibleUeContainers = ['lte-ue*', 'nr-ue*']
			for container in listOfPossibleUeContainers:
				filenames = self.yamlPath[0] + '/*-oai-' + container + '.log'
				cmd = 'ls ' + filenames
				containerStatus = True
				try:
					lsStatus = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=10)
					filenames = str(lsStatus).strip()
				except:
					containerStatus = False
				if not containerStatus:
					continue

				for filename in filenames.split('\n'):
					logging.debug('\u001B[1m Analyzing UE logfile ' + filename + ' \u001B[0m')
					logStatus = UE.AnalyzeLogFile_UE(filename, HTML, RAN)
					if (logStatus < 0):
						fullStatus = False
						HTML.CreateHtmlTestRow('UE log Analysis', 'KO', logStatus)
					else:
						HTML.CreateHtmlTestRow('UE log Analysis', 'OK', CONST.ALL_PROCESSES_OK)

			cmd = 'rm ' + self.yamlPath[0] + '/*.log'
			logging.debug(cmd)
			deployStatus = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=10)
			if self.tsharkStarted:
				self.tsharkStarted = True
				ymlPath = self.yamlPath[0].split('/')
				cmd = 'sudo chmod 666 /tmp/capture_' + ymlPath[1] + '.pcap'
				logging.debug(cmd)
				copyStatus = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=10)
				cmd = 'cp /tmp/capture_' + ymlPath[1] + '.pcap ' + logPath
				logging.debug(cmd)
				copyStatus = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=10)
				cmd = 'sudo rm /tmp/capture_' + ymlPath[1] + '.pcap'
				logging.debug(cmd)
				copyStatus = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=10)
				self.tsharkStarted = False

		logging.debug('\u001B[1m Undeploying \u001B[0m')
		cmd = 'cd ' + self.yamlPath[0] + ' && docker-compose -f docker-compose-ci.yml down'
		logging.debug(cmd)
		try:
			deployStatus = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=100)
		except Exception as e:
			self.exitStatus = 1
			logging.error('Could not undeploy')
			HTML.CreateHtmlTestRow('Could not undeploy', 'KO', CONST.ALL_PROCESSES_OK)
			logging.error('\u001B[1m Undeploying OAI Object(s) FAILED\u001B[0m')
			return

		self.deployedContainers = []
		# Cleaning any created tmp volume
		cmd = 'docker volume prune --force || true'
		logging.debug(cmd)
		deployStatus = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=100)

		if fullStatus:
			HTML.CreateHtmlTestRow('n/a', 'OK', CONST.ALL_PROCESSES_OK)
			logging.info('\u001B[1m Undeploying OAI Object(s) PASS\u001B[0m')
		else:
			HTML.CreateHtmlTestRow('n/a', 'KO', CONST.ALL_PROCESSES_OK)
			logging.info('\u001B[1m Undeploying OAI Object(s) FAIL\u001B[0m')

	def StatsFromGenObject(self, HTML):
		self.exitStatus = 0
		ymlPath = self.yamlPath[0].split('/')
		logPath = '../cmake_targets/log/' + ymlPath[1]

		# if the containers are running, recover the logs!
		cmd = 'cd ' + self.yamlPath[0] + ' && docker-compose -f docker-compose-ci.yml ps --all'
		logging.debug(cmd)
		deployStatus = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=30)
		cmd = 'docker stats --no-stream --format "table {{.Container}}\t{{.CPUPerc}}\t{{.MemUsage}}\t{{.MemPerc}}" '
		anyLogs = False
		for state in deployStatus.split('\n'):
			res = re.search('Name|----------', state)
			if res is not None:
				continue
			if len(state) == 0:
				continue
			res = re.search('^(?P<container_name>[a-zA-Z0-9\-\_]+) ', state)
			if res is not None:
				anyLogs = True
				cmd += res.group('container_name') + ' '
		message = ''
		if anyLogs:
			logging.debug(cmd)
			stats = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=30)
			for statLine in stats.split('\n'):
				logging.debug(statLine)
				message += statLine + '\n'

		html_queue = SimpleQueue()
		html_cell = '<pre style="background-color:white">\n' + message + '</pre>'
		html_queue.put(html_cell)
		HTML.CreateHtmlTestRowQueue(self.pingOptions, 'OK', 1, html_queue)

	def PingFromContainer(self, HTML, RAN, UE):
		self.exitStatus = 0
		ymlPath = self.yamlPath[0].split('/')
		logPath = '../cmake_targets/log/' + ymlPath[1]
		cmd = 'mkdir -p ' + logPath
		deployStatus = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=10)

		cmd = 'docker exec ' + self.pingContName + ' /bin/bash -c "ping ' + self.pingOptions + '" 2>&1 | tee ' + logPath + '/ping_' + HTML.testCase_id + '.log || true'

		logging.debug(cmd)
		deployStatus = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=100)

		result = re.search(', (?P<packetloss>[0-9\.]+)% packet loss, time [0-9\.]+ms', deployStatus)
		if result is None:
			self.PingExit(HTML, RAN, UE, False, 'Packet Loss Not Found')
			return

		packetloss = result.group('packetloss')
		if float(packetloss) == 100:
			self.PingExit(HTML, RAN, UE, False, 'Packet Loss is 100%')
			return

		result = re.search('rtt min\/avg\/max\/mdev = (?P<rtt_min>[0-9\.]+)\/(?P<rtt_avg>[0-9\.]+)\/(?P<rtt_max>[0-9\.]+)\/[0-9\.]+ ms', deployStatus)
		if result is None:
			self.PingExit(HTML, RAN, UE, False, 'Ping RTT_Min RTT_Avg RTT_Max Not Found!')
			return

		rtt_min = result.group('rtt_min')
		rtt_avg = result.group('rtt_avg')
		rtt_max = result.group('rtt_max')
		pal_msg = 'Packet Loss : ' + packetloss + '%'
		min_msg = 'RTT(Min)    : ' + rtt_min + ' ms'
		avg_msg = 'RTT(Avg)    : ' + rtt_avg + ' ms'
		max_msg = 'RTT(Max)    : ' + rtt_max + ' ms'

		message = 'ping result\n'
		message += '    ' + pal_msg + '\n'
		message += '    ' + min_msg + '\n'
		message += '    ' + avg_msg + '\n'
		message += '    ' + max_msg + '\n'
		packetLossOK = True
		if float(packetloss) > float(self.pingLossThreshold):
			message += '\nPacket Loss too high'
			packetLossOK = False
		elif float(packetloss) > 0:
			message += '\nPacket Loss is not 0%'
		self.PingExit(HTML, RAN, UE, packetLossOK, message)

		if packetLossOK:
			logging.debug('\u001B[1;37;44m ping result \u001B[0m')
			logging.debug('\u001B[1;34m    ' + pal_msg + '\u001B[0m')
			logging.debug('\u001B[1;34m    ' + min_msg + '\u001B[0m')
			logging.debug('\u001B[1;34m    ' + avg_msg + '\u001B[0m')
			logging.debug('\u001B[1;34m    ' + max_msg + '\u001B[0m')
			logging.info('\u001B[1m Ping Test PASS\u001B[0m')

	def PingExit(self, HTML, RAN, UE, status, message):
		html_queue = SimpleQueue()
		html_cell = '<pre style="background-color:white">UE\n' + message + '</pre>'
		html_queue.put(html_cell)
		if status:
			HTML.CreateHtmlTestRowQueue(self.pingOptions, 'OK', 1, html_queue)
		else:
			logging.error('\u001B[1;37;41m ping test FAIL -- ' + message + ' \u001B[0m')
			HTML.CreateHtmlTestRowQueue(self.pingOptions, 'KO', 1, html_queue)
			# Automatic undeployment
			logging.debug('----------------------------------------')
			logging.debug('\u001B[1m Starting Automatic undeployment \u001B[0m')
			logging.debug('----------------------------------------')
			HTML.testCase_id = 'AUTO-UNDEPLOY'
			HTML.desc = 'Automatic Un-Deployment'
			self.UndeployGenObject(HTML, RAN, UE)
			self.exitStatus = 1

	def IperfFromContainer(self, HTML, RAN):
		self.exitStatus = 0

		ymlPath = self.yamlPath[0].split('/')
		logPath = '../cmake_targets/log/' + ymlPath[1]
		cmd = 'mkdir -p ' + logPath
		logStatus = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=10)

		# Start the server process
		cmd = 'docker exec -d ' + self.svrContName + ' /bin/bash -c "nohup iperf ' + self.svrOptions + ' > /tmp/iperf_server.log 2>&1" || true'
		logging.debug(cmd)
		serverStatus = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=10)
		time.sleep(5)

		# Start the client process

		cmd = 'docker exec ' + self.cliContName + ' /bin/bash -c "iperf ' + self.cliOptions + '" 2>&1 | tee '+ logPath + '/iperf_client_' + HTML.testCase_id + '.log || true'
		logging.debug(cmd)
		clientStatus = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=100)

		# Stop the server process
		cmd = 'docker exec ' + self.svrContName + ' /bin/bash -c "pkill iperf" || true'
		logging.debug(cmd)
		serverStatus = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=10)
		time.sleep(5)
		cmd = 'docker cp ' + self.svrContName + ':/tmp/iperf_server.log '+ logPath + '/iperf_server_' + HTML.testCase_id + '.log'
		logging.debug(cmd)
		serverStatus = subprocess.check_output(cmd, shell=True, stderr=subprocess.STDOUT, universal_newlines=True, timeout=30)

		# Analyze client output
		result = re.search('Server Report:', clientStatus)
		if result is None:
			result = re.search('read failed: Connection refused', clientStatus)
			if result is not None:
				message = 'Could not connect to iperf server!'
			else:
				message = 'Server Report and Connection refused Not Found!'
			self.IperfExit(HTML, False, message)
			logging.error('\u001B[1;37;41m Iperf Test FAIL\u001B[0m')
			return

		# Computing the requested bandwidth in float
		result = re.search('-b (?P<iperf_bandwidth>[0-9\.]+)[KMG]', self.cliOptions)
		if result is not None:
			req_bandwidth = result.group('iperf_bandwidth')
			req_bw = float(req_bandwidth)
			result = re.search('-b [0-9\.]+K', self.cliOptions)
			if result is not None:
				req_bandwidth = '%.1f Kbits/sec' % req_bw
				req_bw = req_bw * 1000
			result = re.search('-b [0-9\.]+M', self.cliOptions)
			if result is not None:
				req_bandwidth = '%.1f Mbits/sec' % req_bw
				req_bw = req_bw * 1000000

		reportLine = None
		reportLineFound = False
		for iLine in clientStatus.split('\n'):
			if reportLineFound:
				reportLine = iLine
				reportLineFound = False
			res = re.search('Server Report:', iLine)
			if res is not None:
				reportLineFound = True
		result = None
		if reportLine is not None:
			result = re.search('(?:|\[ *\d+\].*) (?P<bitrate>[0-9\.]+ [KMG]bits\/sec) +(?P<jitter>[0-9\.]+ ms) +(\d+\/ ..\d+) +(\((?P<packetloss>[0-9\.]+)%\))', reportLine)
		iperfStatus = True
		if result is not None:
			bitrate = result.group('bitrate')
			packetloss = result.group('packetloss')
			jitter = result.group('jitter')
			logging.debug('\u001B[1;37;44m iperf result \u001B[0m')
			iperfStatus = True
			msg = 'Req Bitrate : ' + req_bandwidth + '\n'
			logging.debug('\u001B[1;34m    Req Bitrate : ' + req_bandwidth + '\u001B[0m')
			if bitrate is not None:
				msg += 'Bitrate     : ' + bitrate + '\n'
				logging.debug('\u001B[1;34m    Bitrate     : ' + bitrate + '\u001B[0m')
				result = re.search('(?P<real_bw>[0-9\.]+) [KMG]bits/sec', str(bitrate))
				if result is not None:
					actual_bw = float(str(result.group('real_bw')))
					result = re.search('[0-9\.]+ K', bitrate)
					if result is not None:
						actual_bw = actual_bw * 1000
					result = re.search('[0-9\.]+ M', bitrate)
					if result is not None:
						actual_bw = actual_bw * 1000000
					br_loss = 100 * actual_bw / req_bw
					if br_loss < 90:
						iperfStatus = False
					bitperf = '%.2f ' % br_loss
					msg += 'Bitrate Perf: ' + bitperf + '%\n'
					logging.debug('\u001B[1;34m    Bitrate Perf: ' + bitperf + '%\u001B[0m')
			if packetloss is not None:
				msg += 'Packet Loss : ' + packetloss + '%\n'
				logging.debug('\u001B[1;34m    Packet Loss : ' + packetloss + '%\u001B[0m')
				if float(packetloss) > float(5):
					msg += 'Packet Loss too high!\n'
					logging.debug('\u001B[1;37;41m Packet Loss too high \u001B[0m')
					iperfStatus = False
			if jitter is not None:
				msg += 'Jitter      : ' + jitter + '\n'
				logging.debug('\u001B[1;34m    Jitter      : ' + jitter + '\u001B[0m')
			self.IperfExit(HTML, iperfStatus, msg)
		else:
			iperfStatus = False
			logging.error('problem?')
			self.IperfExit(HTML, iperfStatus, 'problem?')
		if iperfStatus:
			logging.info('\u001B[1m Iperf Test PASS\u001B[0m')
		else:
			logging.error('\u001B[1;37;41m Iperf Test FAIL\u001B[0m')

	def IperfExit(self, HTML, status, message):
		html_queue = SimpleQueue()
		html_cell = '<pre style="background-color:white">UE\n' + message + '</pre>'
		html_queue.put(html_cell)
		if status:
			HTML.CreateHtmlTestRowQueue(self.cliOptions, 'OK', 1, html_queue)
		else:
			logging.error('\u001B[1m Iperf Test FAIL -- ' + message + ' \u001B[0m')
			HTML.CreateHtmlTestRowQueue(self.cliOptions, 'KO', 1, html_queue)


	def CheckAndAddRoute(self, svrName, ipAddr, userName, password):
		logging.debug('Checking IP routing on ' + svrName)
		mySSH = SSH.SSHConnection()
		if svrName == 'porcepix':
			mySSH.open(ipAddr, userName, password)
			# Check if route to asterix gnb exists
			mySSH.command('ip route | grep --colour=never "192.168.68.64/26"', '\$', 10)
			result = re.search('172.21.16.127', mySSH.getBefore())
			if result is None:
				mySSH.command('echo ' + password + ' | sudo -S ip route add 192.168.68.64/26 via 172.21.16.127 dev eno1', '\$', 10)
			# Check if route to obelix enb exists
			mySSH.command('ip route | grep --colour=never "192.168.68.128/26"', '\$', 10)
			result = re.search('172.21.16.128', mySSH.getBefore())
			if result is None:
				mySSH.command('echo ' + password + ' | sudo -S ip route add 192.168.68.128/26 via 172.21.16.128 dev eno1', '\$', 10)
			# Check if route to nepes gnb exists
			mySSH.command('ip route | grep --colour=never "192.168.68.192/26"', '\$', 10)
			result = re.search('172.21.16.137', mySSH.getBefore())
			if result is None:
				mySSH.command('echo ' + password + ' | sudo -S ip route add 192.168.68.192/26 via 172.21.16.137 dev eno1', '\$', 10)
			# Check if forwarding is enabled
			mySSH.command('sysctl net.ipv4.conf.all.forwarding', '\$', 10)
			result = re.search('net.ipv4.conf.all.forwarding = 1', mySSH.getBefore())
			if result is None:
				mySSH.command('echo ' + password + ' | sudo -S sysctl net.ipv4.conf.all.forwarding=1', '\$', 10)
			# Check if iptables forwarding is accepted
			mySSH.command('echo ' + password + ' | sudo -S iptables -L FORWARD', '\$', 10)
			result = re.search('Chain FORWARD .*policy ACCEPT', mySSH.getBefore())
			if result is None:
				mySSH.command('echo ' + password + ' | sudo -S iptables -P FORWARD ACCEPT', '\$', 10)
			mySSH.close()
		if svrName == 'asterix':
			mySSH.open(ipAddr, userName, password)
			# Check if route to porcepix epc exists
			mySSH.command('ip route | grep --colour=never "192.168.61.192/26"', '\$', 10)
			result = re.search('172.21.16.136', mySSH.getBefore())
			if result is None:
				mySSH.command('echo ' + password + ' | sudo -S ip route add 192.168.61.192/26 via 172.21.16.136 dev em1', '\$', 10)
			# Check if route to porcepix cn5g exists
			mySSH.command('ip route | grep --colour=never "192.168.70.128/26"', '\$', 10)
			result = re.search('172.21.16.136', mySSH.getBefore())
			if result is None:
				mySSH.command('echo ' + password + ' | sudo -S ip route add 192.168.70.128/26 via 172.21.16.136 dev em1', '\$', 10)
			# Check if X2 route to obelix enb exists
			mySSH.command('ip route | grep --colour=never "192.168.68.128/26"', '\$', 10)
			result = re.search('172.21.16.128', mySSH.getBefore())
			if result is None:
				mySSH.command('echo ' + password + ' | sudo -S ip route add 192.168.68.128/26 via 172.21.16.128 dev em1', '\$', 10)
			# Check if forwarding is enabled
			mySSH.command('sysctl net.ipv4.conf.all.forwarding', '\$', 10)
			result = re.search('net.ipv4.conf.all.forwarding = 1', mySSH.getBefore())
			if result is None:
				mySSH.command('echo ' + password + ' | sudo -S sysctl net.ipv4.conf.all.forwarding=1', '\$', 10)
			# Check if iptables forwarding is accepted
			mySSH.command('echo ' + password + ' | sudo -S iptables -L FORWARD', '\$', 10)
			result = re.search('Chain FORWARD .*policy ACCEPT', mySSH.getBefore())
			if result is None:
				mySSH.command('echo ' + password + ' | sudo -S iptables -P FORWARD ACCEPT', '\$', 10)
			mySSH.close()
		if svrName == 'obelix':
			mySSH.open(ipAddr, userName, password)
			# Check if route to porcepix epc exists
			mySSH.command('ip route | grep --colour=never "192.168.61.192/26"', '\$', 10)
			result = re.search('172.21.16.136', mySSH.getBefore())
			if result is None:
				mySSH.command('echo ' + password + ' | sudo -S ip route add 192.168.61.192/26 via 172.21.16.136 dev eno1', '\$', 10)
			# Check if X2 route to asterix gnb exists
			mySSH.command('ip route | grep --colour=never "192.168.68.64/26"', '\$', 10)
			result = re.search('172.21.16.127', mySSH.getBefore())
			if result is None:
				mySSH.command('echo ' + password + ' | sudo -S ip route add 192.168.68.64/26 via 172.21.16.127 dev eno1', '\$', 10)
			# Check if X2 route to nepes gnb exists
			mySSH.command('ip route | grep --colour=never "192.168.68.192/26"', '\$', 10)
			result = re.search('172.21.16.137', mySSH.getBefore())
			if result is None:
				mySSH.command('echo ' + password + ' | sudo -S ip route add 192.168.68.192/26 via 172.21.16.137 dev eno1', '\$', 10)
			# Check if forwarding is enabled
			mySSH.command('sysctl net.ipv4.conf.all.forwarding', '\$', 10)
			result = re.search('net.ipv4.conf.all.forwarding = 1', mySSH.getBefore())
			if result is None:
				mySSH.command('echo ' + password + ' | sudo -S sysctl net.ipv4.conf.all.forwarding=1', '\$', 10)
			# Check if iptables forwarding is accepted
			mySSH.command('echo ' + password + ' | sudo -S iptables -L FORWARD', '\$', 10)
			result = re.search('Chain FORWARD .*policy ACCEPT', mySSH.getBefore())
			if result is None:
				mySSH.command('echo ' + password + ' | sudo -S iptables -P FORWARD ACCEPT', '\$', 10)
			mySSH.close()
		if svrName == 'nepes':
			mySSH.open(ipAddr, userName, password)
			# Check if route to porcepix epc exists
			mySSH.command('ip route | grep --colour=never "192.168.61.192/26"', '\$', 10)
			result = re.search('172.21.16.136', mySSH.getBefore())
			if result is None:
				mySSH.command('echo ' + password + ' | sudo -S ip route add 192.168.61.192/26 via 172.21.16.136 dev enp0s31f6', '\$', 10)
			# Check if X2 route to obelix enb exists
			mySSH.command('ip route | grep --colour=never "192.168.68.128/26"', '\$', 10)
			result = re.search('172.21.16.128', mySSH.getBefore())
			if result is None:
				mySSH.command('echo ' + password + ' | sudo -S ip route add 192.168.68.128/26 via 172.21.16.128 dev enp0s31f6', '\$', 10)
			# Check if forwarding is enabled
			mySSH.command('sysctl net.ipv4.conf.all.forwarding', '\$', 10)
			result = re.search('net.ipv4.conf.all.forwarding = 1', mySSH.getBefore())
			if result is None:
				mySSH.command('echo ' + password + ' | sudo -S sysctl net.ipv4.conf.all.forwarding=1', '\$', 10)
			# Check if iptables forwarding is accepted
			mySSH.command('echo ' + password + ' | sudo -S iptables -L FORWARD', '\$', 10)
			result = re.search('Chain FORWARD .*policy ACCEPT', mySSH.getBefore())
			if result is None:
				mySSH.command('echo ' + password + ' | sudo -S iptables -P FORWARD ACCEPT', '\$', 10)
			mySSH.close()

