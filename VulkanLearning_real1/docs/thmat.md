The material layout follows:

ID (CRC-16)

Color

Roughness

Normals

Displacement

AmbientOcclusion

Metalness


These load the textures to an LveTextures array, and maps it to an ID stored in a hash map, so if a material hasn't been loaded yet, then it loads it, otherwise it reuses it.


Todo:
Make a 1x1 image whenever a material parameter is passed along as a single float. Done for ease of life.
