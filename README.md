<h1>Flatburn-LTE deployment Rijksuniversiteit Groningen</h1>

A visual overview of the Flatburn-LTE system as deployed at the Rijksuniversiteit Groningen.
Swap in your module photos and exploded views below to create a clean, engaging preview.

## At a glance
| Item | Detail |
| --- | --- |
| Deployment site | Rijksuniversiteit Groningen |
| System role | [One-line purpose, e.g., LTE-enabled sensing platform] |
| Key modules | Module 1, Module 2, [add more] |
| Enclosure | [Material / IP rating] |
| Power | [Battery / mains / solar] |
| Status | [Operational / prototype / retired] |
| Last updated | 2026-02-16 |

## System overview
![System overview photo](assets/modules/Module-1.jpg)

Replace this image with a wide system overview photo when available.
Include 2-3 sentences on system intent, environment, and constraints.

## Module gallery
<table>
  <tr>
    <td align="center">
      <img src="assets/modules/Module-1.jpg" width="280" alt="Module 1" />
      <br />
      <sub>Module 1 - [Name]</sub>
    </td>
    <td align="center">
      <img src="assets/modules/Module-2.jpg" width="280" alt="Module 2" />
      <br />
      <sub>Module 2 - [Name]</sub>
    </td>
  </tr>
</table>

One short paragraph explaining how the modules relate and why these are the key building blocks.
Duplicate cells to add more modules.

## Exploded views
<table>
  <tr>
    <td align="center">
      <img src="assets/exploded/exploded.png" width="360" alt="Exploded view" />
      <br />
      <sub>Exploded view - [Enclosure / stack]</sub>
    </td>
  </tr>
</table>

Add a short note about assembly order, fasteners, or serviceability.

## Architecture snapshot
Add a block diagram at `assets/architecture-block-diagram.png`, then uncomment the line below.
<!-- ![Block diagram](assets/architecture-block-diagram.png) -->

2-3 sentences describing the signal flow and data path from sensors to LTE backhaul.

## Repository map
- `hardware/` Hardware design files, CAD, and BOMs.
- `firmware/` Embedded firmware sources and build notes.
- `Version_22_1/` Reference release snapshot for the deployment.
- `assets/` Image assets for this overview page.

## How to update images
1. Drop images into `assets/`, `assets/modules/`, or `assets/exploded/`.
1. Update the filenames in this README to match your assets.
1. Keep a consistent crop ratio and background for a cohesive gallery.
