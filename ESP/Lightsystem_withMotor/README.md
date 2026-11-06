# Lichtsystem mit Motor

## Projektübersicht
Dieses Projekt dient zur Unterstützung der Kunstausstellung von Kazuki Taguchi.
Er steuert die LED-Beleuchtung und den Motor.
Der Motor dreht eine Scheibe, während das flackende LED-Licht eine warme Atmosphäre erzeugt

## Hardware-komponenten
- **Mikrokontroller-Board:**
    - ESP-32
- **Zusatzmodule / Treiber:**
  - DC-DC-Wandler
  - Motortreiber-Modul
- **Aktoren / Ausgangsgeräte:**
  - Schrittmotor (28BYJ-48)
  - LEDs

## Entwicklungsumgebung und Tools
- **Entwicklungsumgebung:** Visual Studio Code
- **Programmiersprache:** C / C++ (ESP-IDF / Arduino Framework)
- **Kommunikation & Schnittstellen:**
  - GPIO (Digital Input / Output)

## Funktionsablauf
1. **LED-Beleuchtung:** Jede LED leuchtet auf individuelle Weise. Durch sanftes Flackern (Flicker-Effekt) entsteht ein warmes, nicht zu helles Licht.
2. **Motorrotation:** Der Motor dreht sich langsam mit konstanter Geschwindigkeit.

#　モーターおよびLED照明制御システム(日本語)

## プロジェクト概要
このプロジェクトは芸術家田口氏の展示補助を目的としている。
LED照明およびモーターの制御を行う。
モーターの動力で円盤を回転させ、LEDの揺らぎおよび演出によって温かい雰囲気にする。

## 使用機器
- **マイコンボード**
    - ESP-32
- **補助ボード**
    - DC-DCコンバータ
    - モーターコントローラー
- **出力機器**
    - モーター　28BYJ-48
    - LED

## 開発環境とツール
**開発環境:** Visual Studio Code
**プログラム言語:** C
**通信規格等:**
    - GPIO(Digital Input/Output)

## 動作の流れ ##
1.**LED点灯**　各LEDがそれぞれ違う光り方をして、明るすぎず温かい光を演出する。そのために揺らぎなどを表現する。
2.**モーター回転**　ゆっくり一定の速度で回転する。
3.
4.