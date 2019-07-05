## RESOLVED: Naming

`pyshade` is not good.  Maybe `libshade` and `shaderbox`/`shaderflat`
front ends.

#### Resolution:
Project is *shaderboy*.
Executables will be **shaderbox** and eventually **shaderflat**.
Library is **libshade**.
C symbol prefix is `SHD_`/`shd_`.

## RESOLVED: info log

Should not be part of prog object, should not force non-const for
`instantiate` and `is_okay`.

#### Resolution:
`prog_is_okay` now has an `info_log` out parameter.
`prog` type is now const in `exec` and `render` modules.

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

## RESOLVED: Implement Textures

#### Resolution:
Implemented textures.

## RESOLVED: Implement Noise

#### Resolution:
Implemented noise.

## RESOLVED: Implement Time

Should implement at least one

#### Resolution:
Implemented iTime and iResolution.

# Implement more predefined variables.

`iTimeDelta`, `iFrame`, `iMouse`, `iDate`, `iChannel` should be easy.


## Implement CPU and IRQ affinity

## Get real-time kernel

## `shd_deinit` should destroy all programs.

## LEDs size is hardcoded in `render.update_predefines`.

## RESOLVED: Implement `shd_fps`.

####Resolution:
Implemented `shd_fps`.

## Locks up if run duration less than 0.
