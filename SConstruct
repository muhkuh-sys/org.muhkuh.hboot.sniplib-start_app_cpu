# -*- coding: utf-8 -*-
#-------------------------------------------------------------------------#
#   Copyright (C) 2016 by Christoph Thelen                                #
#   doc_bacardi@users.sourceforge.net                                     #
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
#   This program is distributed in the hope that it will be useful,       #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#   GNU General Public License for more details.                          #
#                                                                         #
#   You should have received a copy of the GNU General Public License     #
#   along with this program; if not, write to the                         #
#   Free Software Foundation, Inc.,                                       #
#   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             #
#-------------------------------------------------------------------------#


#----------------------------------------------------------------------------
#
# Set up the Muhkuh Build System.
#
SConscript('mbs/SConscript')
Import('atEnv')

# Create a build environment for the Cortex-M4 based netX chips.
env_cortexM4 = atEnv.DEFAULT.CreateEnvironment(['gcc-arm-none-eabi-4.9', 'asciidoc'])
env_cortexM4.CreateCompilerEnv('NETX90_MPW', ['arch=armv7', 'thumb'], ['arch=armv7e-m', 'thumb'])

# Build the platform libraries.
SConscript('platform/SConscript')


#----------------------------------------------------------------------------
#
# Get the source code version from the VCS.
#
atEnv.DEFAULT.Version('#targets/version/version.h', 'templates/version.h')


#----------------------------------------------------------------------------
#
# Build all files.
#
sources = """
	src/header.c
	src/main.c
	src/sha384.c
"""

# The list of include folders. Here it is used for all files.
astrIncludePaths = ['src', '#platform/src', '#platform/src/lib', '#targets/version']

tEnv = atEnv.NETX90_MPW.Clone()
tEnv.Append(CPPPATH = astrIncludePaths)
tEnv.Replace(LDFILE = 'src/netx90/netx90_com_intram.ld')
tSrc = tEnv.SetBuildPath('targets/netx90_com_intram', 'src', sources)
tElf = tEnv.Elf('targets/netx90_com_intram/start_app_cpu_netx90_com_intram.elf', tSrc + tEnv['PLATFORM_LIBRARY'])
tTxt = tEnv.ObjDump('targets/netx90_com_intram/start_app_cpu_netx90_com_intram.txt', tElf, OBJDUMP_FLAGS=['--disassemble', '--source', '--all-headers', '--wide'])
tBin = tEnv.ObjCopy('targets/netx90_com_intram/start_app_cpu_netx90_com_intram.bin', tElf)
tTmp = tEnv.GccSymbolTemplate('targets/netx90_com_intram/snippet.xml', tElf, GCCSYMBOLTEMPLATE_TEMPLATE='templates/hboot_snippet.xml', GCCSYMBOLTEMPLATE_BINFILE=tBin[0])

# Create the snippet from the parameters.
global PROJECT_VERSION
aArtifactGroupReverse = ['org', 'muhkuh', 'hboot', 'sniplib']
atSnippet = {
    'group': '.'.join(aArtifactGroupReverse),
    'artifact': 'start_app_cpu_netx90_mpw',
    'version': PROJECT_VERSION,
    'vcs_id': tEnv.Version_GetVcsIdLong(),
    'vcs_url': tEnv.Version_GetVcsUrl(),
    'license': 'GPL-2.0',
    'author_name': 'Muhkuh team',
    'author_url': 'https://github.com/muhkuh-sys',
    'description': 'Start the APP CPU on a netX90 MPW.',
    'categories': ['netx90MPW', 'booting']
}
strArtifactPath = 'targets/snippets/%s/%s/%s' % ('/'.join(aArtifactGroupReverse), atSnippet['artifact'], PROJECT_VERSION)
snippet_netx90_mpw_com = tEnv.HBootSnippet('%s/%s-%s.xml' % (strArtifactPath, atSnippet['artifact'], PROJECT_VERSION), tTmp, PARAMETER=atSnippet)

# Create the POM file.
tPOM = tEnv.POMTemplate('%s/%s-%s.pom' % (strArtifactPath, atSnippet['artifact'], PROJECT_VERSION), 'templates/pom.xml', POM_TEMPLATE_GROUP=atSnippet['group'], POM_TEMPLATE_ARTIFACT=atSnippet['artifact'], POM_TEMPLATE_VERSION=atSnippet['version'], POM_TEMPLATE_PACKAGING='xml')
