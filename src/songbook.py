#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Import all required modules
import locale
import os.path
import sys
import logging
import threading

# Import patacrep modules
from patacrep.build import SongbookBuilder, DEFAULT_STEPS
from patacrep.utils import yesno
from patacrep import __version__
from patacrep import errors
from patacrep.songbook import open_songbook

# Expose C++ to local python
from PythonQt import *

# Define global variables
sb_builder = None
process = None
stopProcess = False
# logging.basicConfig(level=logging.DEBUG)


# Define locale according to user's parameters
def setLocale():
    try:
        locale.setlocale(locale.LC_ALL, '')
    except locale.Error as error:
        print("Locale Error")
        # Throw error


# Test patacrep version
def testPatacrep():
    return patacrep.__version__


def message(text):
    CPPprocess.message(text, 0)


# Load songbook and setup datadirs
def setupSongbook(songbook_path):
    setLocale()
    global sb_builder

    # Load songbook from sb file.
    try:
        songbook = open_songbook(songbook_path)
        # Setup details in songbook
        songbook['_cache'] = True
        songbook['_error'] = "fix"
    except errors.SongbookError as error:
        print("Error in formation of Songbook Builder")
        print(error)
        # Deal with error

    # Default value
    basename = os.path.splitext(os.path.basename(songbook_path))[0]
    songbook['_basename'] = basename

    try:
        sb_builder = SongbookBuilder(songbook)
        sb_builder.unsafe = True
    except errors.SongbookError as error:
        print("Error in formation of Songbook Builder")
        print(error)
        # Deal with error


# Wrapper around buildSongbook that manages the threading part
def build(steps):
    global stopProcess
    global process
    stopProcess = False
    process = threading.Thread(target=buildSongbook, args=(steps,))
    try:
        # logger.info('Starting process')
        process.start()
    except threading.ThreadError as error:
        # logger.warning('process Error occured')
        message(error)

    period = 2
    while process.is_alive():
        # message("it's alive: " + CPPprocess.getBuildState())
        if not CPPprocess.getBuildState():
            try:
                process.crash()
            except AttributeError as error:
                message("Building exited at user's request")
                raise
        # Check in 2 seconds
        process.join(period)
    message("end build")


# Inner function that actually builds the songbook
def buildSongbook(steps):
    global sb_builder
    # message("Inner Function Reached")
    sys.stdout.flush()
    try:
        for step in steps:
            message("Building songbook: " + step)
            sb_builder.build_steps([step])
        message("Building finished")
    except errors.SongbookError as error:
        message("Building error")
        # Call proper function in CPPprocess
        message(error)
        raise
    # message("Exiting buildSongbook function")


def stopBuild():
    message("Terminating process")
    global stopProcess
    stopProcess = True
