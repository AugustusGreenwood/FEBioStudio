QT_ROOT="~/local/x86_64/homebrew"
run_cmake() {
	cmake -L . -B cmbuild \
		-DQt_Root=$QT_ROOT \
		-DFEBio_SDK=febio4-sdk \
		-DUSE_FFMPEG=ON \
		-DUSE_TETGEN=ON \
		-DUSE_MMG=ON \
		-DUSE_SSH=ON \
		-DCAD_FEATURES=ON \
		-DUSE_ITK=ON \
		-DUSE_NETGEN=OFF \
		-DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 \
		-DCMAKE_OSX_ARCHITECTURES=x86_64
}


