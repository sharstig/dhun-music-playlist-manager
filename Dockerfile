# ==========================================
# Multi-stage Dockerfile for DHUN on Render
# B.Tech CSE Data Structures & Algorithms Project
# ==========================================

# Stage 1: Build the C++17 Backend with CMake & GCC
FROM gcc:12-bookworm AS builder

# Install CMake and build dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake \
    make \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy project files
COPY CMakeLists.txt ./
COPY include/ ./include/
COPY src/ ./src/
COPY tests/ ./tests/

# Build core library, server binary, and unit tests
RUN mkdir build && cd build \
    && cmake -DCMAKE_BUILD_TYPE=Release .. \
    && cmake --build . --config Release \
    && ./dhun_tests

# Stage 2: Minimal Runtime Image
FROM debian:bookworm-slim AS runner

# Install curl for healthcheck and runtime libraries
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    curl \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy compiled binary from builder
COPY --from=builder /app/build/dhun_server ./dhun_server

# Copy frontend static assets (HTML, CSS, Vanilla JS)
COPY public/ ./public/

# Default port configuration (overridden by Render via PORT env var)
ENV PORT=8080
EXPOSE 8080

# Healthcheck endpoint
HEALTHCHECK --interval=30s --timeout=5s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:${PORT:-8080}/health || exit 1

# Execute backend server
CMD ["./dhun_server"]
