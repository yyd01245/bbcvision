#!/bin/bash

COMPILW_PATH=`pwd`

cd  ${COMPILW_PATH}/Switch_Config/

make clean;make static;make install;

cd  ${COMPILW_PATH}/Main/

make clean;make;make install;


