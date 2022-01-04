# JSON Parser example

## Introduction

This is a demo that uses `ParserKit` to create a simple JSON parser. If you
are unfamiliar with the JSON file format you can learn more about it 
[here](https://www.json.org).

At its simplest, a JSON file is composed of a `Value` entity or a collection of 
`Value` entities. Collections are either an array of `Value` entites, or an object. 
Arrays are enclosed by square brackets, as in [value, value, ...].

An object is a collection of zero or more name-`Value` pairs enclosed by curly
braces, as in { name: `Value`, name: `Value`, ...}

A `Value` can be one of the following:
* an Object
* an Array
* a quoted string
* a number
* true
* false
* null

## The JSONValue object

The key element of this example is the JSONValue class. This class encapsulates
the concept of a JSON `Value` in a variant type. After successfully parsing 
a JSON file, the parser returns a JSONValue object that represents the root of
the hierarchy defined by the JSON file.
