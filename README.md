# bfpp
Relay Brainfuck PC computer extended BF++ compiler

# BF Header

```
uint16_t Magic = "BF";
uint16_t ImageBase;//Address Of Image Started
struct{
  uint16_t Pos;//Position of Code segment. After loading IP = Pos
  uint16_t Length;//Length of Segment to load
}Code;
struct{
  uint16_t Pos;//Position of Data segment. After loading AP = Pos
  uint16_t Length;//Length of Segment to load
}Data;
```
