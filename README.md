# HothouseDrumMachine
Simple drum machine code for Hothouse DSP Pedal Kit 


## Installation

Clone the project first:
```
git clone https://github.com/mateuszwojt/HothouseDrumMachine.git
```

Initialize the submodules (this project depends on [libDaisy](https://github.com/electro-smith/libDaisy) and [DaisySP](https://github.com/electro-smith/DaisySP)):
```
cd HothouseDrumMachine
git submodule update --init --recursive
```

Build libDaisy and DaisySP (this may take a while):
```
make -C libDaisy
make -C DaisySP
```

Once that is done, you should be able to go into the effect source code (`src/DrumMachine`) and run `make`.

To install the effect onto the Hothouse pedal, put your Daisy Seed into DFU mode and run:
```
make program-dfu
```

If you're using a ST-Link V3 programmer/debugger probe, run:
```
make program
```

