# IoT-Based Multilingual Text-to-Speech System

## Project Description
This project is an ESP8266 NodeMCU based IoT Text-to-Speech (TTS) system designed to convert user-input text into spoken audio using Wi-Fi connectivity and cloud-based speech synthesis services. The system provides a web-based dashboard where users can type text, select different language accents, and control audio volume.

The NodeMCU communicates with the Google Translate and Google Text-to-Speech APIs to generate multilingual speech output. Audio playback is handled through the MAX98357A I2S digital audio amplifier connected to a speaker. The system uses LittleFS file storage to temporarily store MP3 audio files before playback.

The project is powered by a rechargeable 18650 Li-ion battery and includes a Type-C charging/power module for portability. This makes the device suitable for assistive communication, smart IoT applications, educational demonstrations, and embedded systems learning.

### Hardware Setup
![Hardware1](images/hardware1.jpeg)

![Hardware2](images/hardware2.jpeg)

## Features
- Wi-Fi enabled text-to-speech conversion
- Multilingual translation and speech synthesis
- Web-based dashboard interface
- Adjustable audio volume
- Portable battery-powered design
- MP3 audio playback using I2S interface
- Local MP3 storage using LittleFS
- Real-time speech generation

### Web Dashboard
![Dashboard](images/dashboard.jpeg)

## Technologies Used
- ESP8266 NodeMCU
- Arduino IDE
- HTML/CSS/JavaScript
- Google TTS API
- Google Translate API
- MAX98357A Audio Amplifier
- LittleFS File System
- WiFiManager Library

### circuit-diagram
![circuit](circuit-diagram/Screenshot 2026-05-15 001118.png)

## Applications
- Assistive communication systems
- Smart IoT devices
- Embedded systems projects
- Educational and learning tools
- Multilingual speech systems
