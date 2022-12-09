from skbuild import setup
setup(
    packages=["photon_net"],
    package_dir={"": "python"},
    cmake_install_dir="python/photon_net",
    cmake_args=["-DPYTHON_MODULE=ON", "-DMACOSX_DEPLOYMENT_TARGET=10.15"],
    package_data={"photon_net": ["py.typed", "*.pyi", "**/*.pyi", "*/*/*.pyi"]},
    zip_safe=False,
)
