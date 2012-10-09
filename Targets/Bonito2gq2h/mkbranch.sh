#!/bin/bash
for i in `find ./ -name "*3a2h*"`
do
  #echo $i
  dst=`echo $i | sed -e "s/3a2h/2gq2h/g"`
  #echo $dst
  echo "mv $i to $dst"
  mv $i $dst
  sed -i "s/3a2h/2gq2h/g" $dst
done
