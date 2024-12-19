<p align="center">
    <img src=".github/assets/imgs/spqr-logo-no-bg.png" alt="SPQR" width="400">
    <br>
    <strong>Selective Protocol for Quality and Reliability</strong>
</p>

# spqr

![GitHub repo size](https://img.shields.io/github/repo-size/AntonioBerna/spqr)
![GitHub License](https://img.shields.io/github/license/AntonioBerna/spqr)
![GitHub Created At](https://img.shields.io/github/created-at/antonioberna/spqr)

## Introduction

The aim of the project is to design and implement in `C` language using the Berkeley Socket API a client-server application for file transfer that uses the connectionless network service (socket type `SOCK_DGRAM`, i.e. UDP as transport layer protocol). The software must allow:

- client-server connection without authentication;
- viewing the files available on the server on the client (`LIST` command);
- downloading a file from the server (`GET` command);
- uploading a file to the server (`PUT` command);
- transfer files reliably.

Communication between client and server must occur through an appropriate protocol. The communication protocol must provide for the exchange of two types of messages:

- command messages: they are sent from the client to the server to request the execution of various operations;
- response messages: they are sent from the server to the client in response to a command with the outcome of the operation.

## Server Features

The concurrent server must provide the following features:

- sending the response message to the `LIST` command to the requesting client; the response message contains the filelist, i.e. the list of file names available for sharing;
- sending the response message to the `GET` command containing the requested file, if present, or an appropriate error message;
- receiving a `PUT` message containing the file to upload to the server and sending a response message with the outcome of the operation.

## Client Features

The customer, of the competing type, must provide the following features:

- sending the `LIST` message to request a list of available filenames;
- sending the `GET` message to get a file;
- receiving a file is requested via the `GET` message or handling any errors;
- sending the `PUT` message to upload a file to the server and receiving the response message with the outcome of the operation.

## Reliable Transmission

The exchange of messages occurs using an unreliable communication service. In order to guarantee the correct sending/reception of messages and files, both clients and server implement the `Selective Repeat` protocol with `WINDOW_SIZE` sending window at the application level.

To simulate the loss of messages on the network (a rather unlikely event in a local network not to mention than when client and server are running on the same host), it is assumed that each message is discarded by the sender with probability `LOSS_PROBABILITY`.

The shipping `WINDOW_SIZE`, the message loss probability `LOSS_PROBABILITY`, and the timeout duration `TIMEOUT` are three configurable constants that are the same for all processes. In addition to using a fixed timeout, it must be possible to choose to use an adaptive timeout value that is dynamically calculated based on the evolution of observed network delays.

The client and server must run in user space without requiring root privileges. The server must listen on a default (configurable) port.
