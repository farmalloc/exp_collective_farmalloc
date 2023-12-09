#!/bin/bash

{
  echo "----------"
  cat /proc/cpuinfo

  echo "----------"
  cat /proc/meminfo

  echo "----------"
  uname -a

  echo "----------"
  cat /etc/lsb-release

  echo "----------"
  docker --version

  echo "----------"
} | tee dump_env_result.txt