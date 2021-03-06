#!/usr/bin/python

# Copyright (c) 2008 Johns Hopkins University.
# All rights reserved.
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose, without fee, and without written
# agreement is hereby granted, provided that the above copyright
# notice, the (updated) modification history and the author appear in
# all copies of this source code.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, LOSS OF USE, DATA,
# OR PROFITS) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGE.

# @author Chieh-Jan Mike Liang <cliang4@cs.jhu.edu>
# @author Razvan Musaloiu-E. <razvanm@cs.jhu.edu>

import sys, subprocess
import struct


# ===== STEP 0: Prepares function-ID maps ===== #
map_extfun = {
              "tosthread_sleep":0, "tosthread_create":1,

              "led0On":2, "led1On":3, "led2On":4, 
              "led0Off":5, "led1Off":6, "led2Off":7, 
              "led0Toggle":8, "led1Toggle":9, "led2Toggle":10,

              "amSerialStart":11, "amSerialStop":12, "amSerialReceive":13,
              "amSerialSend":14, "amSerialLocalAddress":15, "amSerialGetLocalGroup":16,
              "amSerialGetDestination":17, "amSerialGetSource":18, "amSerialSetDestination":19,
              "amSerialSetSource":20, "amSerialIsForMe":21, "amSerialGetType":22,
              "amSerialSetType":23, "amSerialGetGroup":24, "amSerialSetGroup":25,
              "serialClear":26, "serialGetPayloadLength":27, "serialSetPayloadLength":28,
              "serialMaxPayloadLength":29, "serialGetPayload":30, "serialRequestAck":31,
              "serialNoAck":32, "serialWasAcked":33,
              
              "amRadioStart":34, "amRadioStop":35, "amRadioReceive":36,
              "amRadioSend":37, "amRadioGetLocalAddress":38, "amRadioGetLocalGroup":39,
              "amRadioGetDestination":40, "amRadioGetSource":41, "amRadioSetDestination":42,
              "amRadioSetSource":43, "amRadioIsForMe":44, "amRadioGetType":45,
              "amRadioSetType":46, "amRadioGetGroup":47, "amRadioSetGroup":48,
              "radioClear":49, "radioGetPayloadLength":50, "radioSetPayloadLength":51,
              "radioMaxPayloadLength":52, "radioGetPayload":53, "radioRequestAck":54,
              "radioNoAck":55, "radioWasAcked":56,
                
              "semaphore_reset":57, "semaphore_acquire":58, "semaphore_release":59,
                
              "barrier_reset":60, "barrier_block":61, "barrier_isBlocking":62,
                
              "condvar_init":63, "condvar_wait":64, "condvar_signalNext":65,
              "condvar_signalAll":66, "condvar_isBlocking":67,
                
              "mutex_init":68, "mutex_lock":69, "mutex_unlock":70,
                
              "volumeBlockRead":71, "volumeBlockWrite":72, "volumeBlockCrc":73,
              "volumeBlockErase":74, "volumeBlockSync":75,
                
              "refcounter_init":76, "refcounter_increment":77, "refcounter_decrement":78,
              "refcounter_waitOnValue":79, "refcounter_count":80,
              
              "amRadioSnoop":81,
              
              "queue_init":82, "queue_clear":83, "queue_push":84,
              "queue_pop":85, "queue_remove":86, "queue_size":87,
              "queue_is_empty":88,
              
              "sensirionSht11_humidity_read":89, "sensirionSht11_humidity_getNumBits":90, "sensirionSht11_temperature_read":91,
              "sensirionSht11_temperature_getNumBits":92,
  
              "hamamatsuS10871_tsr_read":93, "hamamatsuS10871_tsr_readStream":94, "hamamatsuS10871_tsr_getNumBits":95,
  
              "hamamatsuS1087_par_read":96, "hamamatsuS1087_par_readStream":97, "hamamatsuS1087_par_getNumBits":98,
              
              "volumeLogRead":99, "volumeLogCurrentReadOffset":100, "volumeLogSeek":101,
              "volumeLogGetSize":102,
              
              "volumeLogAppend":103, "volumeLogCurrentWriteOffset":104, "volumeLogErase":105,
              "volumeLogSync":106,

              "getLeds":107, "setLeds":108,

              "__divmodhi4":109,
              
              "tosthread_join":110,
              
              "volumeConfigMount":111, "volumeConfigRead":112, "volumeConfigWrite":113,
              "volumeConfigCommit":114, "volumeConfigGetSize":115, "volumeConfigValid":116}

