# Simple Blockchain Implementation in C

This project implements a basic blockchain with a Proof of Work (PoW) consensus mechanism and an HTTP server for interaction. It's designed as a learning tool to understand the fundamental concepts of blockchain technology.

## Features

- **Block Structure**: Implementation of blocks containing transactions, timestamps, proof of work, and previous hash references.
- **Transaction Management**: Creation and validation of transactions between addresses.
- **Proof of Work**: Simple mining algorithm to create new blocks.
- **Chain Validation**: Verification of the blockchain's integrity by checking block links and proofs.
- **HTTP API**: RESTful API to interact with the blockchain.

## API Endpoints

- **POST /transactions/new**: Add a new transaction to the pending list.
- **GET /mine**: Execute the PoW algorithm and create a new block.
- **GET /chain**: Retrieve the full blockchain.

## Project Structure

```
blockchain_project/
│
├── src/
│   ├── main.c              # Main application entry point
│   ├── blockchain.c        # Blockchain implementation
│   ├── blockchain.h        # Blockchain header file
│   ├── block.c             # Block structure implementation
│   ├── block.h             # Block header file
│   ├── transaction.c       # Transaction implementation
│   ├── transaction.h       # Transaction header file
│   ├── pow.c               # Proof of Work implementation
│   ├── pow.h               # PoW header file
│   ├── http_server.c       # HTTP server implementation
│   ├── http_server.h       # HTTP server header file
│   ├── utils.c             # Utility functions
│   └── utils.h             # Utility header file
│
├── lib/                    # External libraries (e.g., mongoose)
│
├── Makefile                # Build configuration
└── README.md               # Project documentation
```

## Prerequisites

- C compiler
- Make
- OpenSSL libraries
- Mongoose HTTP library

## Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/blockchain-project.git
   cd blockchain-project
   ```

2. Install dependencies:
   ```bash
   # For Ubuntu/Debian
   sudo apt-get install libssl-dev
   
   ```

3. Download Mongoose:
   ```bash
   mkdir -p lib
   curl -L https://github.com/cesanta/mongoose/raw/master/mongoose.c -o lib/mongoose.c
   curl -L https://github.com/cesanta/mongoose/raw/master/mongoose.h -o lib/mongoose.h
   ```

4. Build the project:
   ```bash
   make
   ```

## Usage

1. Start the blockchain server:
   ```bash
   ./blockchain
   ```

2. The server starts on port 8000 by default. You can interact with it using curl:

   Add a new transaction:
   ```bash
   curl -X POST http://localhost:8000/transactions/new \
     -H "Content-Type: application/json" \
     -d '{"sender":"address1", "recipient":"address2", "amount":5.0}'
   ```

   Mine a new block:
   ```bash
   curl http://localhost:8000/mine
   ```

   Get the blockchain:
   ```bash
   curl http://localhost:8000/chain
   ```
