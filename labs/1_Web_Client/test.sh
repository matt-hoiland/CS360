#!/bin/bash

for ((i=0; i<20; i++))
do
	./download localhost 1234 /docs/notfound.txt -d
done
