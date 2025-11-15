# fec-nsc-codec

![Build](https://github.com/fujiyama-kota-comm/fec-nsc-codec/actions/workflows/c-cpp.yml/badge.svg)
![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Version](https://img.shields.io/github/v/tag/fujiyama-kota-comm/fec-nsc-codec)

C implementation of **Non-Systematic Convolutional Codes (NSC)** with **Viterbi decoding**
for Forward Error Correction (FEC).
Supports **hard-decision** and **soft-decision (LLR)** decoding.

---

## 📘 Overview

This repository provides a lightweight and modular implementation of
**Non-Systematic Convolutional (NSC) Codes**, including:

- Rate-1/2 convolutional encoder
- 4-state trellis (constraint length 3)
- Hard/soft Viterbi decoder
- Branchless trellis-based implementation
- BER simulation under AWGN

Designed for:

- FEC research
- Wireless communication (5G/6G)
- Embedded systems
- Error-control coding education

---

## 📁 Project Structure

```
fec-nsc-codec
├── src/                 # Encoder/decoder core implementation
├── include/             # Public header files
├── mains/               # Test programs & BER simulation
├── results/             # Generated BER results
├── images/              # BER plots and diagrams
├── python/              # Plotting scripts
├── .github/workflows/   # CI pipeline (GCC build)
├── Makefile             # Build rules
└── README.md            # This document
```

---

## 📑 Features

### ✔ NSC Encoder (Rate 1/2)
- Trellis table–based generation
- Forced termination (tail bits)
- Branchless implementation

### ✔ Viterbi Decoder
- Hard-decision Viterbi (Hamming metric)
- Soft-decision Viterbi (LLR metric)
- Full traceback implementation
- Trellis defined in `trellis.h`

### ✔ AWGN BER Simulation
The program `mains/nsc_ber.c` evaluates BER vs Eb/N0
for both hard- and soft-decision decoding.

---

## 🛠 Build Instructions

### Requirements
- GCC or Clang
- `make`
- Linux / macOS / WSL / MinGW

---

### Build

```sh
make
```

Generated binary:

```
nsc_ber   # BER simulation program
```

Clean build:

```sh
make clean
```

---

## 🚀 Usage Example

Run BER simulation:

```sh
./nsc_ber
```

Example BER result (CSV):

```
results/nsc_ber_data.csv
```

---

## 📉 BER Performance

Example BER curve for rate-1/2 NSC
(4-state Viterbi, AWGN, BPSK):

![BER curve](images/nsc_ber_graph.png)

---

## 📂 Source Code Structure

### src/
| File | Description |
|------|-------------|
| `nsc_encoder.c` | NSC encoder implementation |
| `nsc_decoder.c` | Hard & soft Viterbi decoder |
| `trellis.c` | Next-state & output tables |

### include/
| File | Description |
|------|-------------|
| `nsc_encoder.h` | Encoder API |
| `nsc_decoder.h` | Decoder API |
| `trellis.h` | Trellis constants |

### mains/
| File | Description |
|------|-------------|
| `nsc_ber.c` | BER simulation under AWGN |

---

## 🔒 Confidentiality Notice

All source code in this repository was developed independently
based only on public standards (3GPP) and academic knowledge.
No confidential or proprietary information from any company,
internship, or NDA-protected source is used.

---

## 📜 License

This project is licensed under the **MIT License**.
You may use it for research, education, and commercial applications.

---

## 🤝 Contributing

Pull requests are welcome.
For major changes, please open an issue first.

---

## ⭐ Acknowledgements

Developed as part of research in
**Forward Error Correction (FEC)** and **physical-layer communications**.

If this repository is useful, please consider giving it a ⭐ on GitHub!
