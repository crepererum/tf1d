#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <signal.h>

#include <alsa/asoundlib.h>
#include <libusb-1.0/libusb.h>

#define BUFFER_IN_SIZE 22

struct input_data {
    bool button_browse;
    bool button_capture;
    bool button_quant;
    bool button_reverse;
    bool button_shift;
    bool button_size;
    bool button_sync;
    bool button_type;
    bool pads[4][4];
    unsigned char knobs[4];
    unsigned char selector;
    bool selector_pressed;
    unsigned char sliders[4];
    bool stops[4];
};

static bool volatile running = true;

void int_handler(int _code) {
    running = false;
}

void send_diff(const struct input_data* current, const struct input_data* last, snd_rawmidi_t* midi_to) {
    unsigned char node[3];

    // pads + stops: channel 0, node 60-79
    node[0]=0x90;
    for (unsigned int x = 0; x < 4; ++x) {
        for (unsigned int y = 0; y < 4; ++y) {
            if (current->pads[y][x] != last->pads[y][x]) {
                node[1] = 60 + 4 * y + x;
                node[2] = current->pads[y][x] ? 127 : 0;
                snd_rawmidi_write(midi_to, node, 3);
            }
        }
    }
    for (unsigned int x = 0; x < 4; ++x) {
        if (current->stops[x] != last->stops[x]) {
            node[1] = 76 + x;
            node[2] = current->stops[x] ? 127 : 0;
            snd_rawmidi_write(midi_to, node, 3);
        }
    }

    // knobs, sliders, selector: control channel 0, node 1-9
    node[0]=0xB0;
    for (unsigned int x = 0; x < 4; ++x) {
        if (current->knobs[x] != last->knobs[x]) {
            node[1] = 1 + x;
            node[2] = current->knobs[x] / 2;
            snd_rawmidi_write(midi_to, node, 3);
        }
    }
    for (unsigned int x = 0; x < 4; ++x) {
        if (current->sliders[x] != last->sliders[x]) {
            node[1] = 5 + x;
            node[2] = current->sliders[x] / 2;
            snd_rawmidi_write(midi_to, node, 3);
        }
    }
    if (current->selector != last->selector) {
        node[1] = 9;
        node[2] = current->selector / 2;
        snd_rawmidi_write(midi_to, node, 3);
    }

    // buttons: control channel 0, node 10-18
    node[0]=0xB0;
    if (current->button_browse != last->button_browse) {
        node[1] = 10;
        node[2] = current->button_browse ? 127 : 0;
        snd_rawmidi_write(midi_to, node, 3);
    }
    if (current->button_capture != last->button_capture) {
        node[1] = 11;
        node[2] = current->button_capture ? 127 : 0;
        snd_rawmidi_write(midi_to, node, 3);
    }
    if (current->button_quant != last->button_quant) {
        node[1] = 12;
        node[2] = current->button_quant ? 127 : 0;
        snd_rawmidi_write(midi_to, node, 3);
    }
    if (current->button_reverse != last->button_reverse) {
        node[1] = 13;
        node[2] = current->button_reverse ? 127 : 0;
        snd_rawmidi_write(midi_to, node, 3);
    }
    if (current->button_shift != last->button_shift) {
        node[1] = 14;
        node[2] = current->button_shift ? 127 : 0;
        snd_rawmidi_write(midi_to, node, 3);
    }
    if (current->button_size != last->button_size) {
        node[1] = 15;
        node[2] = current->button_size ? 127 : 0;
        snd_rawmidi_write(midi_to, node, 3);
    }
    if (current->button_sync != last->button_sync) {
        node[1] = 16;
        node[2] = current->button_sync ? 127 : 0;
        snd_rawmidi_write(midi_to, node, 3);
    }
    if (current->button_type != last->button_type) {
        node[1] = 17;
        node[2] = current->button_type ? 127 : 0;
        snd_rawmidi_write(midi_to, node, 3);
    }
    if (current->selector_pressed != last->selector_pressed) {
        node[1] = 18;
        node[2] = current->selector_pressed ? 127 : 0;
        snd_rawmidi_write(midi_to, node, 3);
    }

    int error = snd_rawmidi_drain(midi_to);
    if (error) {
        printf("Midi write error! (%s)", snd_strerror(error));
    }
}

void parse_input(unsigned char* data, struct input_data* input) {
    // unknown: data[0]

    input->pads[0][0] = (data[1] >> 7) & 0x01;
    input->pads[0][1] = (data[1] >> 6) & 0x01;
    input->pads[0][2] = (data[1] >> 5) & 0x01;
    input->pads[0][3] = (data[1] >> 4) & 0x01;

    input->pads[1][0] = (data[1] >> 3) & 0x01;
    input->pads[1][1] = (data[1] >> 2) & 0x01;
    input->pads[1][2] = (data[1] >> 1) & 0x01;
    input->pads[1][3] = (data[1] >> 0) & 0x01;

    input->pads[2][0] = (data[2] >> 7) & 0x01;
    input->pads[2][1] = (data[2] >> 6) & 0x01;
    input->pads[2][2] = (data[2] >> 5) & 0x01;
    input->pads[2][3] = (data[2] >> 4) & 0x01;

    input->pads[3][0] = (data[2] >> 3) & 0x01;
    input->pads[3][1] = (data[2] >> 2) & 0x01;
    input->pads[3][2] = (data[2] >> 1) & 0x01;
    input->pads[3][3] = (data[2] >> 0) & 0x01;


    // unknown: data[3] & 0x01
    // unknown: data[3] & 0x02
    input->selector_pressed = (data[3] >> 2) & 0x01;
    input->button_browse    = (data[3] >> 3) & 0x01;
    input->button_size      = (data[3] >> 4) & 0x01;
    input->button_type      = (data[3] >> 5) & 0x01;
    input->button_reverse   = (data[3] >> 6) & 0x01;
    input->button_shift     = (data[3] >> 7) & 0x01;

    // unknown: data[4] & 0x01
    input->button_capture = (data[4] >> 1) & 0x01;
    input->button_quant   = (data[4] >> 2) & 0x01;
    input->button_sync    = (data[4] >> 3) & 0x01;

    input->stops[0] = (data[4] >> 7) & 0x01;
    input->stops[1] = (data[4] >> 6) & 0x01;
    input->stops[2] = (data[4] >> 5) & 0x01;
    input->stops[3] = (data[4] >> 4) & 0x01;

    input->selector = data[5];

    input->knobs[0] = ((unsigned int)data[ 7] * 255u + (unsigned int)data[ 6]) / 16u;
    input->knobs[1] = ((unsigned int)data[ 9] * 255u + (unsigned int)data[ 8]) / 16u;
    input->knobs[2] = ((unsigned int)data[11] * 255u + (unsigned int)data[10]) / 16u;
    input->knobs[3] = ((unsigned int)data[13] * 255u + (unsigned int)data[12]) / 16u;

    input->sliders[0] = ((unsigned int)data[15] * 255u + (unsigned int)data[14]) / 16u;
    input->sliders[1] = ((unsigned int)data[17] * 255u + (unsigned int)data[16]) / 16u;
    input->sliders[2] = ((unsigned int)data[19] * 255u + (unsigned int)data[18]) / 16u;
    input->sliders[3] = ((unsigned int)data[21] * 255u + (unsigned int)data[20]) / 16u;
}

int main() {
    int error;

    // setup libusb
    if (libusb_init(NULL)) {
        printf("Cannot initialize libusb!\n");
        return EXIT_FAILURE;
    }

    // get Traktor F1
    libusb_device_handle* handle = libusb_open_device_with_vid_pid(NULL, 0x17cc, 0x1120);
    if (!handle) {
        printf("Device not found or no permissions!\n");
        libusb_exit(NULL);
        return EXIT_FAILURE;
    }

    // eliminate kernel driver and claim the device
    libusb_set_auto_detach_kernel_driver(handle, 1);
    error = libusb_claim_interface(handle, 0);
    if (error) {
        printf("Cannot claim input interface! (%s)\n", libusb_error_name(error));
        libusb_close(handle);
        libusb_exit(NULL);
        return EXIT_FAILURE;
    }

    // search endpoint address
    libusb_device* dev = libusb_get_device(handle);
    struct libusb_config_descriptor* config;
    error = libusb_get_active_config_descriptor(dev, &config);
    if (error) {
        printf("Cannot claim input interface! (%s)\n", libusb_error_name(error));
        libusb_close(handle);
        libusb_exit(NULL);
        return EXIT_FAILURE;
    }
    uint8_t endpoint_address_in = config->interface[0].altsetting[0].endpoint[0].bEndpointAddress;
    libusb_free_config_descriptor(config);

    // setup midi
    snd_rawmidi_t* midi_to;
    snd_rawmidi_t* midi_from;
    error = snd_rawmidi_open(&midi_from, &midi_to, "virtual", 0);
    if (error) {
        printf("Cannot open virtual midi ports! (%s)\n", snd_strerror(error));
        libusb_close(handle);
        libusb_exit(NULL);
        return EXIT_FAILURE;
    }

    // setup interrupt handler
    signal(SIGINT, int_handler);

    // main loop
    unsigned char input_buffer[BUFFER_IN_SIZE];
    int size_transfered;
    struct input_data last;
    while (running) {
        error = libusb_bulk_transfer(handle, endpoint_address_in, input_buffer, BUFFER_IN_SIZE, &size_transfered, 500);
        if (error && error != LIBUSB_ERROR_TIMEOUT) {
            printf("Transfer error! (%s)\n", libusb_error_name(error));
        }

        if (error == LIBUSB_ERROR_NO_DEVICE) {
            printf("Shut down.\n");
            running = false;
        }

        if (size_transfered == 22) {
            struct input_data next;
            parse_input(input_buffer, &next);
            send_diff(&next, &last, midi_to);
            last = next;
        }
    }

    // clean up
    snd_rawmidi_close(midi_to);
    snd_rawmidi_close(midi_from);

    libusb_release_interface(handle, 0);
    libusb_close(handle);
    libusb_exit(NULL);

    // done
    return EXIT_SUCCESS;
}
