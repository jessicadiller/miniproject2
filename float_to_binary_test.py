

def fractional_float_to_binary(num):
    frac_num = num-int(num)
    exponent=0
    shifted_num=frac_num
    while shifted_num != int(shifted_num) and exponent<16:
        shifted_num*=2
        exponent+=1
    # print(shifted_num)
    # print(int(shifted_num))
    new_shifted_num = shifted_num*(2**(16-exponent))
    # if exponent==0:
    #     return '{0:0b}'.format(int(shifted_num))
    # binary='{0:0{1}b}'.format(int(shifted_num),exponent+1)
    # integer_part=binary[:-exponent]
    # fractional_part=binary[-exponent:].rstrip('0')
    # sig_digs='{0}'.format(fractional_part)
    # if len(sig_digs)<16:
    #     bin_str = sig_digs + '0'*(16-len(sig_digs))
    # else:
    #     bin_str= sig_digs[:16]
    # print(binary)
    # print(bin_str)
    return (new_shifted_num)

fractional_float_to_binary(0.1)
fractional_float_to_binary(1.75)
fractional_float_to_binary(0.5)
fractional_float_to_binary(0.25)
(fractional_float_to_binary(0.4))
bin(fractional_float_to_binary(0.3))
int(fractional_float_to_binary(.001))



