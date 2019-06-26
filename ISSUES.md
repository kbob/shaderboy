## Naming

`pyshade` is not good.  Maybe `libshade` and `shaderbox`/`shaderflat`
front ends.

#### Resolution:

Project is shaderboy.
Executables will be shaderbox and eventuall shaderflat.
Library is libshade.
C symbol prefix is `SHD_`/`shd_`.

## info log

Should not be part of prog object, should not force non-const for
`instantiate` and `is_okay`.

## Cleanup

`shd_deinit` does not return.
I think some resources are not released.

## Need GIT.

Need git. Depends on naming.

## BCM cleanup

`bcm_context` vs `vidocore_context` is silly.

