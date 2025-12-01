#!/bin/bash

make && ./demo &&\
        file=$(ls -1t -- *.ppm 2>/dev/null | head -n1) &&\
        [ -n "$file" ] &&\
        sxiv "$file"

