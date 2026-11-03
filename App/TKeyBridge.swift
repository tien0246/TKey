import Foundation

@_silgen_name("tk_setup")          public func tk_setup()
@_silgen_name("tk_cleanup")        public func tk_cleanup()
@discardableResult @_silgen_name("tk_process_char")   public func tkProcessChar(_ keyCode: UInt32) -> UInt32
@discardableResult @_silgen_name("tk_put_char")       public func tkPutChar(_ keyCode: Int32) -> Int32
@discardableResult @_silgen_name("tk_put_backspace")  public func tkPutBackspace() -> Int32
@discardableResult @_silgen_name("tk_reset_buff")     public func tkResetBuff(_ code: Int32) -> Int32
@_silgen_name("tk_set_input_method")    public func tk_set_input_method(_ method: Int32)
@_silgen_name("tk_set_output_charset")  public func tk_set_output_charset(_ charset: Int32)
@_silgen_name("tk_enable_vietnamese")   public func tk_enable_vietnamese(_ enable: Int32)
@_silgen_name("tk_set_option")          public func tk_set_option(_ key: UnsafePointer<CChar>, _ value: Int32)
@_silgen_name("tk_get_buff_len")        public func tk_get_buff_len() -> Int32
@_silgen_name("tk_get_backspace_len")   public func tk_get_backspace_len() -> Int32
@_silgen_name("tk_get_out_type")        public func tk_get_out_type() -> Int32
@_silgen_name("tk_get_buff")            public func tk_get_buff(_ buf: UnsafeMutablePointer<UInt8>, _ len: UnsafeMutablePointer<Int32>)
@_silgen_name("tk_is_vietnamese_enabled") public func tk_is_vietnamese_enabled() -> Bool
@_silgen_name("tk_load_macro")     public func tk_load_macro(_ path: UnsafePointer<CChar>) -> Int32
@_silgen_name("tk_save_macro")     public func tk_save_macro(_ path: UnsafePointer<CChar>) -> Int32
@_silgen_name("tk_macro_add")      public func tk_macro_add(_ key: UnsafePointer<CChar>, _ value: UnsafePointer<CChar>) -> Int32
@_silgen_name("tk_macro_remove")   public func tk_macro_remove(_ key: UnsafePointer<CChar>) -> Int32
@_silgen_name("tk_macro_count")    public func tk_macro_count() -> Int32
@_silgen_name("tk_macro_get")      public func tk_macro_get(_ index: Int32, _ key: UnsafeMutablePointer<CChar>, _ keySize: Int32, _ value: UnsafeMutablePointer<CChar>, _ valueSize: Int32)
@_silgen_name("tk_convert")        public func tk_convert(_ from: Int32, _ to: Int32, _ input: UnsafePointer<UInt8>, _ inSize: Int32, _ output: UnsafeMutablePointer<UInt8>, _ outSize: UnsafeMutablePointer<Int32>) -> Int32
