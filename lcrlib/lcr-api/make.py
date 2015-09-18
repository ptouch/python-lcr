#!/usr/bin/python

import subprocess
import platform

def main():
	current_os = platform.system()
	if current_os == 'Linux':
		subprocess.call(["make", "-f", "Makefile_Linux"])

	if current_os == 'Windows':
		pass


if __name__ == '__main__':
	main()
