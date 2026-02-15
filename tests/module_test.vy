module vcore;
module vmem;
module CustomModule;

sub::CustomModule call(){
    out("hi");
}

out(vmem.usage(CustomModule.call));