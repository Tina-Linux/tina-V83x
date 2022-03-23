This repository is designated to serve as a feed for OpenWrt
(http://openwrt.org) based systems.  As such, the organization, management,
and usage of this repository will be different from the other source
repositories hosted by the AllSeen Alliance.

Most notable of these differences is that the master branch only contains this
readme file.  Package definitions will only exist in branches whos name
corresponds to code names for OpenWrt releases.  At the time of the initial
commit of this readme file, there will be 2 such branches: attitude_adjustment
and barrier_breaker.  As the OpenWrt project develops and creates new
releases, new corresponding branches will be created.

The reason for tracking package definitions on a per OpenWrt release basis is
due to the fact that AllJoyn packages depend on OpenWrt packages which may
change from release to release.  This also allows us to retire support for
obsolete versions of OpenWrt in the future without breaking others who may
still be using/building the obsolete versions.

To use this feed in an OpenWrt build environment, follow these instructions:

1. Copy feeds.conf.default to feeds.conf (if not already done):

cp feeds.conf.default feeds.conf


2. Add the following feeds.conf (replace <openwrt_release> with the
appropriate branch that corresponds to the OpenWrt release you are building):

src-git alljoyn git://git.allseenalliance.org/gerrit/core/openwrt_feed;<openwrt_release>

# For Attitude Adjustment
src-git alljoyn git://git.allseenalliance.org/gerrit/core/openwrt_feed;attitude_adjustment

# For Barrier Breaker
src-git alljoyn git://git.allseenalliance.org/gerrit/core/openwrt_feed;barrier_breaker


3. Update the feed information:

./scripts/feeds update -a


4. Add the the packages from the AllJoyn feed to build system:

./scripts feeds install -a -p alljoyn

5. Enable AllJoyn in the build:

make menuconfig

	Networking --->
		< > alljoyn --->
			< > alljoyn-about
			< > alljoyn-c
			< > alljoyn-config
				< > alljoyn-config-samples
			< > alljoyn-controlpanel
				< > alljoyn-controlpanel-samples
			< > alljoyn-notification
				< > alljoyn-notification-samples
			< > alljoyn-onboarding
				< > alljoyn-onboarding-samples
			< > alljoyn-sample_apps
			< > alljoyn-samples
			< > alljoyn-service_common
