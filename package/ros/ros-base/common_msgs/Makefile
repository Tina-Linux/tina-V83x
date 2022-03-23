#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=common_msgs
PKG_VERSION:=jade

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-devel.zip

PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_LICENSE:=BSD

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)-devel

PKG_BUILD_DEPENDS:=catkin gencpp geneus gennodejs genlisp genpy std_msgs message_runtime roscpp_core

include $(BUILD_DIR)/package.mk
$(call include_mk, genpy-msgsrv.mk)
$(call include_mk, gencpp-msgsrv.mk)

define Package/$(PKG_NAME)
  SUBMENU:=ros-base
  SECTION:=ROS
  CATEGORY:=ROS
  DEPENDS:=+catkin +gencpp +geneus +gennodejs +genlisp +genpy +std_msgs +message_runtime +roscpp_core
  TITLE:=ros common_msgs
  URL:=http://wiki.ros.org/common_msgs?distro=kinetic
endef

define Package/$(PKG_NAME)/description
Commonly used messages in ROS. Includes messages for actions (actionlib_msgs), diagnostics (diagnostic_msgs), geometric primitives (geometry_msgs), robot navigation (nav_msgs), and common sensors (sensor_msgs), such as laser range finders, cameras, point clouds.
endef

sensor_msgs_MSGFILES:= \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/BatteryState.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/CameraInfo.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/ChannelFloat32.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/CompressedImage.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/FluidPressure.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/Illuminance.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/Image.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/Imu.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/JointState.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/JoyFeedbackArray.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/JoyFeedback.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/Joy.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/LaserEcho.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/LaserScan.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/MagneticField.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/MultiDOFJointState.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/MultiEchoLaserScan.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/NavSatFix.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/NavSatStatus.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/PointCloud2.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/PointCloud.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/PointField.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/Range.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/RegionOfInterest.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/RelativeHumidity.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/Temperature.msg \
	$(PKG_BUILD_DIR)/sensor_msgs/msg/TimeReference.msg

sensor_srvs_MSGFILES:= \
	$(PKG_BUILD_DIR)/sensor_msgs/srv/SetCameraInfo.srv

geometry_msgs_MSGFILES:= \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/Accel.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/AccelStamped.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/AccelWithCovariance.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/AccelWithCovarianceStamped.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/Inertia.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/InertiaStamped.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/Point32.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/Point.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/PointStamped.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/Polygon.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/PolygonStamped.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/Pose2D.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/PoseArray.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/Pose.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/PoseStamped.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/PoseWithCovariance.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/PoseWithCovarianceStamped.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/Quaternion.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/QuaternionStamped.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/Transform.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/TransformStamped.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/Twist.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/TwistStamped.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/TwistWithCovariance.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/TwistWithCovarianceStamped.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/Vector3.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/Vector3Stamped.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/Wrench.msg \
	$(PKG_BUILD_DIR)/geometry_msgs/msg/WrenchStamped.msg

actionlib_msgs_MSGFILES:= \
	$(PKG_BUILD_DIR)/actionlib_msgs/msg/GoalID.msg \
	$(PKG_BUILD_DIR)/actionlib_msgs/msg/GoalStatusArray.msg \
	$(PKG_BUILD_DIR)/actionlib_msgs/msg/GoalStatus.msg

stereo_msgs_MSGFILES:= \
	$(PKG_BUILD_DIR)/stereo_msgs/msg/DisparityImage.msg

nav_msgs_MSGFILES:= \
	$(PKG_BUILD_DIR)/nav_msgs/msg/GridCells.msg \
	$(PKG_BUILD_DIR)/nav_msgs/msg/MapMetaData.msg \
	$(PKG_BUILD_DIR)/nav_msgs/msg/OccupancyGrid.msg \
	$(PKG_BUILD_DIR)/nav_msgs/msg/Odometry.msg \
	$(PKG_BUILD_DIR)/nav_msgs/msg/Path.msg

nav_srvs_MSGFILES:= \
	$(PKG_BUILD_DIR)/nav_msgs/srv/GetMap.srv \
	$(PKG_BUILD_DIR)/nav_msgs/srv/GetPlan.srv \
	$(PKG_BUILD_DIR)/nav_msgs/srv/SetMap.srv

define Build/Compile
	@echo -e "\n************** Converts ROS .msg files in a package into Python source code **************\n"
	$(foreach pmsg,$(sensor_msgs_MSGFILES),$(call Build/Compile/MsgToPy,,sensor_msgs,msg, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg -I sensor_msgs:$(PKG_BUILD_DIR)/sensor_msgs/msg \
			-I geometry_msgs:$(PKG_BUILD_DIR)/geometry_msgs/msg,$(pmsg)))

	@echo -e "\n************** Converts ROS .msg files in a package into Python source code initpy **************\n"
	$(foreach pmsg,$(sensor_msgs_MSGFILES),$(call Build/Compile/MsgToPy,,sensor_msgs,msg, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg -I sensor_msgs:$(PKG_BUILD_DIR)/sensor_msgs/msg \
			-I geometry_msgs:$(PKG_BUILD_DIR)/geometry_msgs/msg, --initpy True $(pmsg)))
	$(foreach pmsg,$(sensor_msgs_MSGFILES),$(call Build/Compile/MsgToPy,,sensor_msgs,, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg -I sensor_msgs:$(PKG_BUILD_DIR)/sensor_msgs/msg \
			-I geometry_msgs:$(PKG_BUILD_DIR)/geometry_msgs/msg, --initpy True $(pmsg)))

	@echo -e "\n************** Converts ROS .msg files in a package into C++ source code **************\n"
	$(foreach pmsg,$(sensor_msgs_MSGFILES),$(call Build/Compile/MsgSrvToH,,sensor_msgs,include/sensor_msgs, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg -I sensor_msgs:$(PKG_BUILD_DIR)/sensor_msgs/msg \
			-I geometry_msgs:$(PKG_BUILD_DIR)/geometry_msgs/msg, $(pmsg),,))
	$(foreach psrv,$(sensor_srvs_MSGFILES),$(call Build/Compile/MsgSrvToH,,sensor_msgs,include/sensor_msgs, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg -I sensor_msgs:$(PKG_BUILD_DIR)/sensor_msgs/msg \
			-I geometry_msgs:$(PKG_BUILD_DIR)/geometry_msgs/msg, $(psrv),,))

	@echo -e "\n************** Converts ROS .msg files in a package into Python source code **************\n"
	$(foreach pmsg,$(geometry_msgs_MSGFILES),$(call Build/Compile/MsgToPy,,geometry_msgs,msg, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg -I geometry_msgs:$(PKG_BUILD_DIR)/geometry_msgs/msg,$(pmsg)))
	$(foreach pmsg,$(geometry_msgs_MSGFILES),$(call Build/Compile/MsgToPy,,geometry_msgs,msg, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg -I geometry_msgs:$(PKG_BUILD_DIR)/geometry_msgs/msg, --initpy True $(pmsg)))
	$(foreach pmsg,$(geometry_msgs_MSGFILES),$(call Build/Compile/MsgToPy,,geometry_msgs,, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg -I geometry_msgs:$(PKG_BUILD_DIR)/geometry_msgs/msg, --initpy True $(pmsg)))

	$(foreach pmsg,$(geometry_msgs_MSGFILES),$(call Build/Compile/MsgSrvToH,,geometry_msgs,include/geometry_msgs, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg -I geometry_msgs:$(PKG_BUILD_DIR)/geometry_msgs/msg, $(pmsg),,))

	@echo -e "\n************** Converts ROS .msg files in a package into C++ source code **************\n"
	$(foreach pmsg,$(actionlib_msgs_MSGFILES),$(call Build/Compile/MsgSrvToH,,actionlib_msgs,include/actionlib_msgs, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg -I actionlib_msgs:$(PKG_BUILD_DIR)/actionlib_msgs/msg, $(pmsg),,))

	@echo -e "\n************** Converts ROS .msg files in a package into C++ source code **************\n"
	$(foreach pmsg,$(stereo_msgs_MSGFILES),$(call Build/Compile/MsgSrvToH,,stereo_msgs,include/stereo_msgs, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg  -I sensor_msgs:$(PKG_BUILD_DIR)/sensor_msgs/msg \
			-I stereo_msgs:$(PKG_BUILD_DIR)/stereo_msgs/msg, $(pmsg),,))

	@echo -e "\n************** Converts ROS .msg files in a package into C++ source code **************\n"
	$(foreach pmsg,$(nav_msgs_MSGFILES),$(call Build/Compile/MsgSrvToH,,nav_msgs,include/nav_msgs, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg -I sensor_msgs:$(PKG_BUILD_DIR)/sensor_msgs/msg \
			-I geometry_msgs:$(PKG_BUILD_DIR)/geometry_msgs/msg -I nav_msgs:$(PKG_BUILD_DIR)/nav_msgs/msg, $(pmsg),,))
	$(foreach psrv,$(nav_srvs_MSGFILES),$(call Build/Compile/MsgSrvToH,,nav_msgs,include/nav_msgs, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg -I sensor_msgs:$(PKG_BUILD_DIR)/sensor_msgs/msg \
			-I geometry_msgs:$(PKG_BUILD_DIR)/geometry_msgs/msg -I nav_msgs:$(PKG_BUILD_DIR)/nav_msgs/msg, $(psrv),,))

	cd $(PKG_BUILD_DIR) && \
	$(STAGING_DIR)/usr/bin/catkin_make_isolated --install \
		--source=$(PKG_BUILD_DIR) \
		--build=$(PKG_BUILD_DIR)/build_isolated \
		-DCATKIN_DEVEL_PREFIX=$(PKG_BUILD_DIR)/devel \
		-DCATKIN_ENABLE_TESTING=OFF \
		-DCMAKE_INSTALL_PREFIX=$(PKG_INSTALL_DIR) \
		-DCMAKE_C_COMPILER="$(TARGET_CC)" \
		-DCMAKE_CXX_COMPILER="$(TARGET_CXX)" \
		-DCMAKE_PREFIX_PATH="$(STAGING_DIR)/usr;$(PKG_INSTALL_DIR)"
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/share/* $(1)/usr/share
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/share/* $(1)/usr/share
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
