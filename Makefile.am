#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

include $(top_srcdir)/build/plugins.mk

pkglib_LTLIBRARIES = rediscache_filter.la

if BUILD_HAVE_LIBCXX
AM_CXXFLAGS += -stdlib=libc++
endif

AM_CXXFLAGS += -Wno-undefined
AM_CXXFLAGS += -Wunused-parameter
AM_CXXFLAGS += -Wno-deprecated

rediscache_filter_la_SOURCES = rediscache_filter.cc dictionary.cc iniparser.cc credis.c

rediscache_filter_la_LDFLAGS = $(TS_PLUGIN_LDFLAGS)
