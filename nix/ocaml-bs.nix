{ stdenv, fetchzip }:
stdenv.mkDerivation rec {
  version = "3.1.0";
  shortname = "bucklescript";
  name = "${shortname}-${version}";
  src =
    let tarball = fetchzip {
          url = "https://github.com/BuckleScript/bucklescript/archive/${version}.tar.gz";
          sha256 = "1q128wmnfask720a80zfv2yzaaaa655p3zd8qfckmmhrk7sdcqnd";
        };
     in "${tarball}/vendor/ocaml";
  configurePhase = ''
    ./configure -prefix $out
  '';
  buildPhase = ''
    make world.opt
    '';
}
