The data is hidden in the streaming bits in the top/right corner. You could just concat
all of the bits together to get the proper resulting data, however viewing it as Base64
may help.

If you convert each row of 6 bits and use it as an index to the following string you
will generate a base64 string. This string is the same string used for encoding base64
so it was expected that it would be an easy match.

ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/

At the very end of the right column are two odd dots off to the side, they symbolize
the = sign that base64 doesn't have a symbol for.

Knowing that the two odd dots symbolize the end, just attach the right side stream to
the left side stream. After decoding the base64 data you get a paq8l file. It's
obvious you are on the right path as the beginning of a paq8l header is 'paq8l' which
helps validate the base64 conversion when testing it as a theory. 

The paq8l file will decompress to a large unicode blob from the Halflife Wiki about
G-Man however it has odd spacing and characters. If you treat every ascii character
as a 0 and unicode character as a 1 then you have a new bitstream.

The new bitstream reveals another paq8l file. This file gives you G-man quotes.
Treat each ascii space as 0 and a unicode space as 1. This gives the final answer.

