import machine
import sdcard
import uos
import time
import _thread
import hashlib


SPI_PERIPH = 0
CS_PIN = 29
SCK_PIN = 6
MOSI_PIN = 3
MISO_PIN = 4
SD_BAUD = 25000000

UART_PERIPH = 1
UART_TX_PIN = 20
UART_RX_PIN = 5
UART_BAUD = 230400

BLOCK_SIZE = 16
WRITE_CHUNK_SIZE = 8192
CHUNK_SIZE = 1*WRITE_CHUNK_SIZE
FILE_TIME = 5 # seconds

DO_ENCRYPT = False
ENCRYPT_KEY = ""

# TODO: This is effectively just a shim. need 
# to implement proper asymmetric-key encryption
# with the key stored on the SD card. This will
# ensure that the key used for decryption is not
# stored anywhere on the embedded device.
def encrypt(plaintext, iv):
    if DO_ENCRYPT:
        cipher = hashlib.sha256(ENCRYPT_KEY+iv).digest()
        ciphertext = bytearray([plaintext[i] ^ cipher[i % BLOCK_SIZE]
                      for i in range(len(plaintext))])
        return iv, ciphertext
    else:
        return iv, plaintext

def panic(reason):
    while True:
        print("PANIC:", reason)
        time.sleep(1)

def setup_sdcard():
    cs = machine.Pin(CS_PIN, machine.Pin.OUT)
    spi = machine.SPI(SPI_PERIPH, baudrate=SD_BAUD,
        polarity=0, phase=0, bits=8,
        firstbit=machine.SPI.MSB,
        sck=machine.Pin(SCK_PIN),
        mosi=machine.Pin(MOSI_PIN),
        miso=machine.Pin(MISO_PIN))

    sd = sdcard.SDCard(spi, cs)

    vfs = uos.VfsFat(sd)
    uos.mount(vfs, "/sd")

def get_next_index():
    cur_idx = None
    try:
        with open("/sd/index.txt", "r") as f:
            cur_idx = int(f.read().strip())
    except:
        pass

    if cur_idx:
        try:
            with open("/sd/index.txt", "w+") as f:
                f.write(str(cur_idx+1))
            return cur_idx+1
        except:
            panic("Failed to increment index")
    else:
        try:
            with open("/sd/index.txt", "w+") as f:
                f.write(str(1))
            return 1
        except:
            panic("Failed to create new index file")

def get_next_filename():
    idx = get_next_index()
    fn = "/sd/gps{:06d}.txt".format(idx)

    print("Starting file", fn)
    with open(fn, "w+") as f:
        f.write("GPS")

    return fn

def setup_all():
    global uart
    uart = machine.UART(UART_PERIPH, baudrate=UART_BAUD,
                        tx=machine.Pin(UART_TX_PIN),
                        rx=machine.Pin(UART_RX_PIN))
    uart.init(bits=8, parity=None, stop=1)

    setup_sdcard()


shared_buffer = bytearray()
shared_lock = _thread.allocate_lock()

def uart_thread():
    global uart

    dat_pending = bytearray()
    while True:
        time.sleep_us(10)
        dat = uart.read()
        if dat:
            dat_pending.extend(dat)

        if len(dat_pending):
            if shared_lock.acquire(0):
                shared_buffer.extend(dat_pending)
                dat_pending = bytearray()
                shared_lock.release()

        if len(dat_pending) > 4096:
            with shared_lock:
                shared_buffer.extend(dat_pending)
                dat_pending = bytearray()

def sdcard_thread():
    global shared_lock, shared_buffer

    last_write = 0

    cur_file = get_next_filename()

    while True:
        with shared_lock:
            l = len(shared_buffer)

        # No TOCTOU here because uart_thread only ever
        # extends shared_buffer, never shortens it
        if (l > CHUNK_SIZE) or ((l > BLOCK_SIZE) and (time.time() - last_write > FILE_TIME)):
            _start = time.ticks_ms()

            with shared_lock:
                n = len(shared_buffer)
                n = n - (n % BLOCK_SIZE)
                tmp_buffer = shared_buffer[:n]
                shared_buffer = shared_buffer[n:]

            with open(cur_file, "a+") as f:
                for i in range(0, len(tmp_buffer), WRITE_CHUNK_SIZE):
                    f.write(encrypt(tmp_buffer[i:i+WRITE_CHUNK_SIZE], cur_file)[1])

            # Cycle out the files
            if time.time() - last_write > FILE_TIME:
                cur_file = get_next_filename()
                last_write = time.time()

            print("WRITE TIME", time.ticks_ms() - _start)

# Startup routine
setup_all()
_thread.start_new_thread(uart_thread, ())
sdcard_thread()
