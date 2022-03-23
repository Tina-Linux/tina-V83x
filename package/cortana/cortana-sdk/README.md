# Cortana Device SDK

## Prerequisites

- Have installed the [Companion App](./companion_app) on your Windows 10 PC;

- Ensure your device and Windows 10 PC are both connected to the Internet and on the same network, so that they can ping each other and the following commands should be run successfully:
> ping login.live.com
> curl https://login.live.com

- Ensure the audio path on your device is turned on and it can play and record;

## Usage

1. On your device, run the sample application of Cortana (*cortana-basic-first-sample*).

2. On your Windows 10 PC, run the Companion App and send token.

3. Then if you see the error message "failed to handle HTTP POST request" on your device, please close the sample application and restart it. It should then fetch all the tokens it needs. If it got the OAuth tokens successfully you could see a bunch of success messages.

4. Everything is done! Now you can enjoy the Cortana on your device.
