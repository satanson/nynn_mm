#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements. See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership. The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License. You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied. See the License for the
# specific language governing permissions and limitations
# under the License.
#
vpath %.cpp ../test ../src ../../provider/src ../../producer/src

BOOST_DIR = /usr/include/boost
THRIFT_DIR = /usr/local/include/thrift
LIB_DIR = /usr/local/lib
NYNN_MM_INC= ../../../inc/
AGENT_DIR=../src/
PROVIDER_DIR=../../provider/src
PRODUCER_DIR=../../producer/src

CXXFLAGS =--std=c++0x \
		  -I${THRIFT_DIR} -I${BOOST_DIR}  -I. -L${LIB_DIR} -I${NYNN_MM_INC} -I${AGENT_DIR}\
		  -I${PROVIDER_DIR} -I${PRODUCER_DIR} -lthrift -lthriftnb -levent

GEN_SRC = Agent_constants.cpp Agent.cpp Agent_types.cpp\
		  Producer_constants.cpp Producer.cpp Producer_types.cpp\
		  Provider_constants.cpp Provider.cpp Provider_types.cpp

default:agent 

agent: AgentDaemon.cpp $(GEN_SRC)
	g++ $(CXXFLAGS) -o $@ $^ 
clean:
	$(RM) -r agent 

