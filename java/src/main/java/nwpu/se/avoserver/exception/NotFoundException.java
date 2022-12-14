package nwpu.se.avoserver.exception;

/**
 * 资源不存在异常
 * 会返回HTTP 404
 */
public class NotFoundException extends RuntimeException {

    public NotFoundException(String message) {
        super(message);
    }
}
