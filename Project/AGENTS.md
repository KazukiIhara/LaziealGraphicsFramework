## Project Direction

- This project is a C++ / DirectX 12 graphics framework.
- The API should be approachable for beginners, taking usability cues from OpenSiv3D.
- Pipeline creation should avoid hardcoding where possible and move toward JSON-driven configuration.
- Graphics, window, pipeline, and other framework settings should be externalized where practical.
- The framework should evolve ideas from the previous 3DRobotAction engine.

## Lessons From 3DRobotAction Rendering Review

- No fatal issue was found where the same model is unintentionally color-rendered twice during normal use.
- Opaque objects were rendered in the GBuffer pass with `BlendMode::None`.
- Transparent objects were rendered in the Scene pass with `BlendMode::Normal` or later blend modes.
- Shadow pass rendering was separate depth-only rendering, not duplicate color rendering.
- There was a RenderTarget count mismatch in the GBuffer path: actual `OMSetRenderTargets` and pixel shader outputs used Albedo / Normal only, while the PSO declared 3 render targets.
- Depth resources may have been reused as normal DSVs after being transitioned for SRV reads; use a separate read-only DSV where needed.
- `DispatchMesh` placed `instanceCount` in the Y dimension; the next framework should add limit checks, batch splitting, and `baseInstanceIndex` support.
- Transparent objects were not distance-sorted, so overlapping order could break and appear like duplicate rendering.
- The next engine should introduce concepts such as `RenderGraph`, `RenderPass`, `PipelineKey`, `DrawItem` sorting, and a shared `DispatchMesh` wrapper.
