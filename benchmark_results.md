# Face Detection Performance Benchmark

**Input Video**: `video1.mp4`

| Metric | Decoupled Pipeline | In-Memory Pipeline |
|--------|-------------------|--------------------|
| **Execution Time (s)** | 7.335954507 | 3.291094818 |
| **Intermediate Disk I/O (MB)** | 3.37 | 0.00 |
| **Total Faces Detected** | 63 | 69 |

## Conclusion
- **Speed**: The In-Memory pipeline was 2.22x faster.
- **I/O Efficiency**: The In-Memory pipeline completely eliminated the 3.37 MB of disk reads/writes required by the decoupled approach.
