



## Build


```
make
```


## Auto Format

```

astyle \
--suffix=none \
--style=kr \
--indent=spaces=4 \
--pad-oper \
--pad-header \
--pad-comma \
--unpad-paren \
--unpad-brackets \
--align-pointer=name \
--align-reference=name \
--max-code-length=160 \
--break-after-logical \
--lineend=windows \
--convert-tabs \
--verbose \
--add-braces \
./src/tjpgd.c
```
