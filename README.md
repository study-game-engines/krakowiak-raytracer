# Engine1
Video: [https://youtu.be/itoMzP_KG6M](https://www.youtube.com/watch?v=k279fLso-hE)

The aim of this project is to create a renderer which almost matches Path Tracing in quality, while generating noiseless images hundreds times faster. 
Instead of tracing thousands of rays per pixel to get soft shadows and diffuse reflections, it uses "intelligent" blur in screen space. 
It also uses standard Deferred Rendering for rasterizing main image. One of the project's fundamental rules is that everything is dynamic (no pre-baked data).

<b>Working features:</b><br>
&bull; Deferred Rendering <br>
&bull; Light reflections/refractions <br>
&emsp;&emsp;&middot; Fully dynamic <br>
&emsp;&emsp;&middot; Diffuse - for rough surfaces <br>
&emsp;&emsp;&middot; Multiple refractions/reflections <br>
&emsp;&emsp;&middot; Blur level depanding on surface to object distance <br>
&bull; Shadows <br>
&emsp;&emsp;&middot; Fully dynamic <br>
&emsp;&emsp;&middot; Soft - depanding on light source radius, distance to occluder <br>
&emsp;&emsp;&middot; "Unfinite " precision - tiny/huge objects cast proper soft shadows, light source can be 1000 km away<br>
&emsp;&emsp;&middot; Visible in reflections/refractions <br>
&emsp;&emsp;&middot; Correct shadows from transparent objects - depanding on alpha texture <br>
&bull; Physically Based Rendering - only one material, which supports all effects <br>
&bull; Loading/saving scenes, animations, meshes, generating BVH trees <br>
&bull; Tonemapping, bloom, FXAA, ASSAO (from Intel) <br>

<b>Future plans: </b><br>
&bull; Switching from DX11 do DX12 - to allow for indexing meshes inside shaders, support for huge scenes <br>
&bull; Adjustable visual quality through tracing a few more rays per pixel - like 9, reducing the need for heavy blur <br> 
&bull; Dynamic Global Illumination <br>
&bull; Support for skeletal animation <br>
&bull; Support for arbitrary animation - ex. based on physics - cloths, fluids <br>
&bull; Dynamic caustics <br>
&bull; Volumetric effects, patricles - smoke, fire, clouds <br>
&bull; Creating a sandbox game for testing <br>
