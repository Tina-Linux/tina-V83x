/*
 * Copyright (C) 2014 ALLWINNERTECH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __BLUETOOTH_PARAMS_H__
#define __BLUETOOTH_PARAMS_H__

#define DEVICE_NAME_LEN  256

struct bluetooth_params{
    char device[DEVICE_NAME_LEN+1];
};

void get_params_from_config(const char *config, struct bluetooth_params *bt_params);

#endif
