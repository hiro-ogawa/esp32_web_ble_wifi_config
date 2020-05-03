'use strict';

var vConsole = new VConsole();

const UUID_ESP32_SERVICE = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
const UUID_ESP32_CHARACTERISTIC_1 = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';
const UUID_ESP32_CHARACTERISTIC_2 = 'beb5483f-36e1-4688-b7f5-ea07361b26a8';

var esp32_device = null;
var esp32_chars = new Map();

const encoder = new TextEncoder();

var vue_options = {
  el: "#top",
  data: {
    progress_title: '',

    esp32_name: "MyESP32",
    wifi_ssid: '',
    wifi_password: '',
    wifi_mask: 'this is the wifi mask.',
    esp32_scaned: false,
    esp32_connected: false,
  },
  computed: {
  },
  methods: {
    onDisconnect: function (event) {
      console.log('onDisconnect');
      this.esp32_connected = false;
    },

    esp32_scan: function () {
      return navigator.bluetooth.requestDevice({
        filters: [{ name: this.esp32_name }],
        optionalServices: [UUID_ESP32_SERVICE]
      })
        .then(device => {
          console.log("requestDevice OK");
          console.log(device);

          esp32_device = device;
          esp32_device.addEventListener('gattserverdisconnected', this.onDisconnect);
          this.esp32_scaned = true;
        })
        .catch(error => {
          console.log(error);
        });
    },
    esp32_connect: function () {
      if (esp32_device.connected)
        return;

      return esp32_device.gatt.connect()
        .then(server => {
          console.log('Execute : getPrimaryService');
          return server.getPrimaryService(UUID_ESP32_SERVICE);
        })
        .then(service => {
          console.log('Execute : getCharacteristic');
          return Promise.all([
            set_characteristic(service, UUID_ESP32_CHARACTERISTIC_1),
            set_characteristic(service, UUID_ESP32_CHARACTERISTIC_2),
          ]);
        })
        .then(() => {
          this.esp32_connected = true;
          console.log('Connected!!');
        })
        .catch(error => {
          console.log(error);
        });
    },
    esp32_write: async function () {
      await esp32_chars.get(UUID_ESP32_CHARACTERISTIC_1).writeValue(encoder.encode(this.wifi_ssid));

      var buffer = encoder.encode(this.wifi_password);
      var mask = encoder.encode(this.wifi_mask);
      for (var i = 0; i < buffer.length && i < mask.length; i++)
        buffer[i] ^= mask[i];
      await esp32_chars.get(UUID_ESP32_CHARACTERISTIC_2).writeValue(buffer);

      alert('wrote!');
    },
  },
  created: function () {
  },
  mounted: function () {
    proc_load();
  }
};
vue_add_methods(vue_options, methods_utils);
var vue = new Vue(vue_options);

function set_characteristic(service, characteristicUuid) {
  return service.getCharacteristic(characteristicUuid)
    .then(characteristic => {
      console.log('setCharacteristic : ' + characteristicUuid);
      esp32_chars.set(characteristicUuid, characteristic);
      return service;
    });
}