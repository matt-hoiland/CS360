#!/bin/bash

for ((i=0; i<20; i++))
    do
      GET -dUe http://localhost:1234/docs/test4.jpg &
    done
