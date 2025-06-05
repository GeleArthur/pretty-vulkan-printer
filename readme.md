# pretty vulkan printer

> Arthur van den Barselaar


https://github.com/Howest-DAE-GD/graphics-programming-2-vulkan-project-Arthur-van-den-Barselaar


https://github.com/user-attachments/assets/a15b8aac-ff3d-45bb-8977-f636ff13b1ac


Vulkan deferred renderer. 
Created for graphics programming 2 DAE howest.


### Features
- load models with assimp
- load images using stbc_image
- Depth buffer prepass
- GBuffer render to 2 images
  - Albedo (RGBA)
  - Normal(RG), Roughness(B) Metal(A).
- Light pass. (Cook-Torrance BRDF)
  - point light
  - Directional light
- Post processing
  - Tone mapping (ACES)
  - Exposure 
- HDR to LDR

# Passes

prepass Depth buffer
![depth buffer](.github/image.png)

Gpass albedo
![alebdo](.github/image-1.png)

G pass normals RG
![normals](.github/image-2.png)

G pass roughness (B)
![rougness](.github/image-3.png)

G pass metal (A)
![metal](.github/image-4.png)

G pass (RGB)
![together](.github/image-5.png)

Light pass
![lightpass](.github/image-6.png)

Post processing (Exposure + tone mapping)
![tonemapping](.github/image-7.png)

Final result
![result](.github/image-8.png)
