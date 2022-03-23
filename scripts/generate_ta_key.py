#!/usr/bin/python

def get_args():
        from argparse import ArgumentParser

        parser = ArgumentParser()
        parser.add_argument('--rotpk', required=True, help='path of rotpk.bin file')
        parser.add_argument('--out', required=True, help='Name of out file')
        return parser.parse_args()

def main():
        args = get_args()
        f=open(args.rotpk,'r')
        rotpk = f.read()
        f.close()

        msg = "DERIVE ONE KEY FOR PREPROCESSING".encode('ascii')

        from Crypto.Hash import MD5
        h = MD5.new()
        h.update(rotpk)
        h.update(msg)
        print(h.hexdigest())

        a=h.digest()
        b=a[0:h.digest_size:2]
        f=open(args.out,'wb')
        f.write(a)
        f.close()

if __name__ == "__main__":
        main()
