# CanoKey ESP32-S2
CanoKey is an open-source USB/NFC security token, providing the following functions:

OpenPGP Card V3.4 (RSA, ECDSA, ED25519 supported)
PIV Card
TOTP / HOTP (RFC6238 / RFC4226)
U2F
FIDO2 (WebAuthn)
It works on modern Linux/Windows/macOS operating systems without additional driver required.

THERE IS ABSOLUTELY NO SECURITY ASSURANCE OR WARRANTY USING THE ESP32 VERSION.

ANYONE WITH PHYSICAL ACCESS CAN RETRIEVE ANY DATA (INCLUDING SECRET KEYS) FROM THE KEY.

IT IS ALSO PRONE TO SIDE-CHANNEL ATTACKS.

YOU MUST USE IT AT YOUR OWN RISK.

A SECURE VERSION CAN BE FOUND AT https://canokeys.org

# Harardware
This CanoKey-ESP32-S2 implementation is based on ESP32-S2-FH2 MCU, which features a RISC-V processor, 2MB SPI Flash, 256 KiB SRAM, and a USB controller.

# Build
*I assume you are using Windows env, But Linux works in the same way.*


1. Download this repo

2. Setup the ESP-IDF environment

3. Setup config
```powershell
idf.py set-target esp32s2
idf.py menuconfig
```

You shall config the program i such a way:
> In `Partition Table` you should select `Custom Partition Table CSV`

4. Build and flash
```powershell
idf.py build
idf.py -p COMx flash
```

You shall change the COM port to the actual COM port.

## Notice
You may found that CC1 treats all warning as errors, you may need to add some initial value to the var in canokey-core submodule.