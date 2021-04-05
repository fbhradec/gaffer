#!/bin/bash


act --env GITHUB_PATH=/tmp/xx -s GITHUB_TOKEN=$GAFFER_GITHUB_TOKEN -s GITHUB_REF="$(git branch | grep '*' | awk '{print $2}')"

