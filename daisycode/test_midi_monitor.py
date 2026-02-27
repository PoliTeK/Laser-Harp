#!/usr/bin/env python3
"""
MIDI Monitor for Daisy Seed LaserHarp
Displays incoming MIDI messages from USB
"""

import mido
import time
from datetime import datetime

def list_ports():
    """List all available MIDI ports"""
    print("\n=== Available MIDI Input Ports ===")
    ports = mido.get_input_names()
    if not ports:
        print("No MIDI input ports found!")
        return None
    
    for i, port in enumerate(ports):
        print(f"{i}: {port}")
    return ports

def monitor_midi(port_name):
    """Monitor MIDI messages from specified port"""
    print(f"\n=== Monitoring MIDI from: {port_name} ===")
    print("Press Ctrl+C to stop\n")
    print("Time       | Type      | Note | Velocity | Channel")
    print("-" * 60)
    
    try:
        with mido.open_input(port_name) as inport:
            for msg in inport:
                timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
                
                if msg.type == 'note_on':
                    beam = msg.note - 60  # Assuming base note is 60 (C4)
                    print(f"{timestamp} | NOTE ON   | {msg.note:3d} | {msg.velocity:3d}      | {msg.channel:2d}  <- Beam {beam}")
                
                elif msg.type == 'note_off':
                    beam = msg.note - 60
                    print(f"{timestamp} | NOTE OFF  | {msg.note:3d} | {msg.velocity:3d}      | {msg.channel:2d}  <- Beam {beam}")
                
                else:
                    print(f"{timestamp} | {msg.type:9s} | {str(msg)}")
    
    except KeyboardInterrupt:
        print("\n\nMonitoring stopped.")
    except Exception as e:
        print(f"\nError: {e}")

def main():
    print("=" * 60)
    print("   Daisy Seed LaserHarp - MIDI Monitor")
    print("=" * 60)
    
    ports = list_ports()
    if not ports:
        print("\nMake sure Daisy Seed is connected via USB!")
        return
    
    # Try to auto-select Daisy Seed port
    daisy_port = None
    for port in ports:
        if 'daisy' in port.lower():
            daisy_port = port
            break
    
    if daisy_port:
        print(f"\nAuto-selected: {daisy_port}")
        monitor_midi(daisy_port)
    else:
        print("\nEnter port number to monitor (or 'q' to quit): ", end='')
        choice = input().strip()
        
        if choice.lower() == 'q':
            return
        
        try:
            port_index = int(choice)
            if 0 <= port_index < len(ports):
                monitor_midi(ports[port_index])
            else:
                print("Invalid port number!")
        except ValueError:
            print("Invalid input!")

if __name__ == "__main__":
    main()
