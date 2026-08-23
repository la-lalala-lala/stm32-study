import os
import time

Import("env")


def release_ch340_bootloader(source, target, env):
    """Keep Warship V4's CH340 one-key circuit out of system boot mode."""
    try:
        import serial
        from serial.tools import list_ports
    except ImportError:
        print("CH340 boot release skipped: pyserial is unavailable")
        return

    monitor_port = env.GetProjectOption("monitor_port", "")
    candidates = []
    if monitor_port:
        candidates.append(monitor_port)
    else:
        for port in list_ports.comports():
            if port.vid == 0x1A86 and port.pid in (0x5523, 0x7523):
                candidates.append(port.device)

    if len(candidates) != 1:
        if len(candidates) > 1:
            print("CH340 boot release skipped: set monitor_port when multiple CH340 ports exist")
        return

    try:
        with serial.Serial(candidates[0], 115200, timeout=0) as uart:
            uart.dtr = True
            uart.rts = False
            time.sleep(0.1)
        print("CH340 boot circuit: normal run state (%s)" % candidates[0])
    except (OSError, serial.SerialException) as error:
        print("CH340 boot release skipped for %s: %s" % (candidates[0], error))


platform = env.PioPlatform()
package_dir = platform.get_package_dir("tool-openocd")
scripts_dir = os.path.join(package_dir, "openocd", "scripts")
stlink_config = os.path.join(scripts_dir, "interface", "stlink.cfg")

env.AddPreAction("upload", release_ch340_bootloader)

try:
    with open(stlink_config, encoding="utf-8") as config_file:
        native_stlink = "adapter driver st-link" in config_file.read()
except OSError as error:
    print("ST-Link upload auto-detection failed; using PlatformIO defaults: %s" % error)
    native_stlink = False

if native_stlink:
    executable = "openocd.exe" if os.name == "nt" else "openocd"
    openocd = os.path.join(package_dir, "bin", executable)
    target = env.BoardConfig().get("debug.openocd_target", "stm32f1x")

    env.Replace(
        UPLOADER='"%s"' % openocd,
        UPLOADERFLAGS=[
            "-d1",
            "-s", scripts_dir,
            "-f", "interface/stlink.cfg",
            "-c", "transport select swd",
            "-f", "target/%s.cfg" % target,
            "-c", "program {$SOURCE} verify reset; shutdown;",
        ],
        UPLOADCMD="$UPLOADER $UPLOADERFLAGS",
    )
    print("ST-Link upload transport: native SWD")
else:
    print("ST-Link upload transport: PlatformIO default HLA")
