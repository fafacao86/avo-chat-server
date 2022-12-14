package nwpu.se.avoserver.protocol;



import java.io.FileOutputStream;

public class PipeOutputBean {
    private FileOutputStream pipeOutput;

    public void setPipeOutput(FileOutputStream pipeOutput) {
        this.pipeOutput = pipeOutput;
    }

    public FileOutputStream getPipeOutput() {
        return pipeOutput;
    }

    public PipeOutputBean() {
    }
}
