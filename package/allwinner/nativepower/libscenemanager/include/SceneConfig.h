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

#ifndef __SCENSE_CONFIG_H__
#define __SCENSE_CONFIG_H__
#include <tina_log.h>

#if defined SUN3IW1P1
#include "SUN3IW1P1.h"
#elif defined SUN8IW8P1 	/* sun3iw1p1 define end */
#include "SUN8IW8P1.h"
#elif defined SUN8IW5P1 	/* sun8iw8p1 define end */
#include "SUN8IW5P1.h"
#elif defined SUN8IW6P1		/* sun8iw5p1 define end */
#include "SUN8IW6P1.h"
#elif defined SUN8IW11P1	/* sun8iw6p1 define end */
#include "SUN8IW11P1.h"
#elif defined SUN8IW15P1	/* sun8iw11p1 define end */
#include "SUN8IW15P1.h"
#elif defined SUN50IW1P1	/* sun8iw15p1 define end */
#include "SUN50IW1P1.h"
#elif defined SUN50IW3P1	/* sun50iw1p1 define end */
#include "SUN50IW3P1.h"
#endif				/* sun50iw3p1 define end */

#endif /* file define end */
