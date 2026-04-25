# Face Detection Performance Benchmark

**Input Video**: `video1.mp4`

| Metric | Decoupled Pipeline | In-Memory Pipeline |
|--------|-------------------|--------------------|
| **Execution Time (s)** | 11.475177284 | 6.837557369 |
| **Intermediate Disk I/O (MB)** | 3.37 | 0.00 |
| **Total Faces Detected** | 63 | 69 |

## Conclusion
- **Speed**: The In-Memory pipeline was 1.67x faster.
- **I/O Efficiency**: The In-Memory pipeline completely eliminated the 3.37 MB of disk reads/writes required by the decoupled approach.
