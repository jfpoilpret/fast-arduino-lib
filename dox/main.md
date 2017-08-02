FastArduino Documentation	{#index}
=========================

Welcome to FastArduino documentation!

FastArduino is a C++ object-oriented library for Arduino boards based on AVR MCU and also bare AVR MCU. FastArduino:

- provides smaller and faster code than other libraries for the same functionality
- ensures you pay (size and speed) only for what you use
- uses Object-Oriented Design
- reduces risk of bad code by performing extensive compile-time checks
- supports event-driven programs
- supports both ATmega and ATtiny chips

FastArduino implementation:
- is based on C++11 standard
- favours C++ templates rather than virtual methods whenever possible
- virtual methods are used only when needed (mostly for event handlers)
- never automatically registers any ISR but requires explicit registration through provided macros

The rest of this guide provides indications where to start and proceed with learning how to use FastArduino for your projects.

Building a project using FastArduino
------------------------------------
The easiest way to start a project using FastArduino is probably to use [netbeans](TODO), first setup as described [here](TODO).

Then you can create a new C++ project in netbeans, set the proper configuration for it, based on your target MCU or board, and include FastArduino library to the project.

Your project shall have at least one ".cpp" source file with an `int main()` function, as shown in the [tutorial](TODO).

In the generated `Makefile` of the project, you can include FastArduino provided `Makefile-FastArduino.mk` which will help setup of all predefined macros. Take a look at comments in this provided Makefile to understand how to use it.

Once you can build your project from netbeans, you can build it from a Linux shell as well:

> $ make CONF=UNO-Release
> 
> $ make flash CONF=UNO-Release

TODO: copy from Conway example readme!

FastArduino library organisation
--------------------------------
TODO explain namespaces, templates, macros for ISR, directories.

Discovering FastArduino API step by step
----------------------------------------
TODO link to tutorial

