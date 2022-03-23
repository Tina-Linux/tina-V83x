#!/usr/bin/env python
#
# Copyright 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
#
# You may not use this file except in compliance with the terms and conditions set forth in the
# accompanying LICENSE.TXT file.
#
# THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS.  AMAZON SPECIFICALLY DISCLAIMS, WITH RESPECT TO
# THESE MATERIALS, ALL WARRANTIES, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.

"""
Applies the AVS SDK Adapter for Wakeword Lite files to the AVS SDK.
"""

from __future__ import print_function
from os import path
import subprocess
import sys

# A string to search on CHANGELOG.md to check if the patch will be applied to a compatible version
# of the AVS SDK. We assume that we only add contents to the CHANGELOG.md file, so this string
# specifies the first version of the SDK that is compatible with this patch.
MINIMUM_AVS_SDK_VERSION = '### v1.9.0 released 08/28/2018:'


def exit_with_error(message, code):
    """
    Prints a message and calls system.exit
    :param message: The message to print
    :param code: The exit code
    """

    sys.stderr.write(message + '\n')
    sys.exit(code)


def main(args):
    """
    Main method
    :param args: System arguments - There should be two arguments. The first is the script name and
    the second should be the AVS SDK directory path.
    """

    if len(args) != 2:
        exit_with_error('Usage: %s <avs_sdk_directory>' % args[0], 1)

    avs_sdk_dir = args[1]
    if not path.exists(avs_sdk_dir):
        exit_with_error('Error: %s does not exist.' % avs_sdk_dir, 2)
    if not path.isdir(avs_sdk_dir):
        exit_with_error('Error: %s is not a directory.' % avs_sdk_dir, 3)

    changelog_file = path.join(avs_sdk_dir, 'CHANGELOG.md')
    if not path.exists(changelog_file):
        exit_with_error('Error: %s does not exist.' % changelog_file, 4)
    if not path.isfile(changelog_file):
        exit_with_error('Error: %s is not a file.' % changelog_file, 5)

    if MINIMUM_AVS_SDK_VERSION not in open(changelog_file).read():
        exit_with_error('Error: The string "%s" was not found on %s. It seems you are trying to '
                        'apply this patch on an incompatible version of the SDK.' %
                        (MINIMUM_AVS_SDK_VERSION, changelog_file), 6)

    print('Applying patch to ' + avs_sdk_dir)

    dir_tree = path.dirname(path.realpath(__file__))
    # We should implement this recursive copy in Python. There is no out-of-the-box way to do it
    # using the standard libs.
    subprocess.call(['bash', '-c', 'cp -rf %s/avs-cpp-sdk/* %s' % (dir_tree, avs_sdk_dir)])

    print('Done!')

    print('''
You can now proceed to creating a build directory, running cmake from it, compiling, running unit
tests, and running the sample application.

You should have an Amazon Wakeword distribution artifact and set a variable to point to its
location on step 3 below. These instructions are for a Linux distribution of the Amazon Wakeword
adapter. Please make the necessary changes for other distributions.

Also, please check the AVS SDK documentation (https://github.com/alexa/avs-device-sdk/wiki) for
instructions regarding steps 4 and 9.

1. mkdir -p /workspace/BUILD
2. cd /workspace/BUILD
3. AMAZON_LITE_RELEASE_DIR="<amazon_lite_release_location>"
4. CMAKE_ARGUMENTS="<add_arguments>"
5. Follow the instructions on instructions.txt to add flags and configure the Sample App.
6. cmake \\
    -DAMAZONLITE_KEY_WORD_DETECTOR=ON \\
    -DAMAZONLITE_KEY_WORD_DETECTOR_LIB_PATH=<path-to-lib> \\
    -DAMAZONLITE_KEY_WORD_DETECTOR_INCLUDE_DIR=<path-to-directory-including-the-header-file> \\
    -DAMAZONLITE_KEY_WORD_DETECTOR_DYNAMIC_MODEL_LOADING=OFF \\
    ${CMAKE_ARGUMENTS} \\
    %s
7. make all
8. make test
9. cd ./SampleApp/src/
10. ./SampleApp -L DEBUG9 -C ../../Integration/AlexaClientSDKConfig.json
''' % path.abspath(avs_sdk_dir))


if __name__ == "__main__":
    main(sys.argv)
