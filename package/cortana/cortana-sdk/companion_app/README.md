# Test Companion App

This is a demo UWP (Universal Windows Platform) application that will allow you to log in with your MSA once you have the Cortana SDK up and running.

This application is mandatory for testing a Cortana device (currently it is the only way to get the correct user token needed to authenticate with Cortana).

## Prerequisites

- User should have MSA account, and should already be a Cortana user;

- Make sure you are in Developer mode. (Settings -> Update & security -> For developers -> Developer mode)

## Install

1. Uninstall your old version app firstly if you have one on your Desktop;

2. Unzip TestCompanionApp.zip;

3. Install certification: double click "MusicCompanion_nnnn.cer", install it to "Local Machine" and place it in "Trusted People" store;

4. Right click "App-AppDevPackage.ps1", select "Run with PowerShell", and the app will be installed.

## Usage

1. Ensure the Cortana device and your Windows 10 PC is connected to the same network (so that they can ping each other);

2. Use the following command in the Windows Run prompt (Win + R) to launch the app in debug IP mode:
> cortana-device-setup://sendtokenviaip

3. Walk through all steps of OOBE, then select "Send Token by IP";

4. Select "http" protocol, input the IP address of your Cortana Device, use port 12345, and then push "Send Token".
