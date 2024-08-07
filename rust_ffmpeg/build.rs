fn main() {
    println!("cargo:rustc-link-lib=Mfplat");
    println!("cargo:rustc-link-lib=Strmiids");
    println!("cargo:rustc-link-lib=Mfuuid");
    println!("cargo:rustc-link-lib=Bcrypt");
    println!("cargo:rustc-link-lib=Secur32");
    println!("cargo:rustc-link-lib=Ole32");
    println!("cargo:rustc-link-lib=User32");
}
