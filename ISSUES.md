## RESOLVED: Naming

`pyshade` is not good.  Maybe `libshade` and `shaderbox`/`shaderflat`
front ends.

#### Resolution:

Project is *shaderboy*.
Executables will be **shaderbox** and eventually **shaderflat**.
Library is **libshade**.
C symbol prefix is `SHD_`/`shd_`.

## info log

Should not be part of prog object, should not force non-const for
`instantiate` and `is_okay`.

## RESOLVED: Cleanup

`shd_deinit` does not return.

#### Resolution:

Stopped using `pthread_cancel`; modified `check_cancel` to detect
shutdown and exit thread.

## Cleanup

I think some resources are not released.

## RESOLVED: Need GIT.

Need git. Depends on naming.

#### Resolution:

Has git.  https://github.com/kbob/shaderboy

## BCM cleanup

`bcm_context` vs `vidocore_context` is silly.

## Implement Textures

## Implement Time

#### Resolution:

Implemented iTime and iResolution.

## Implement CPU and IRQ affinity

## Get real-time kernel
