use Packet;

use byteorder::{LittleEndian, ReadBytesExt, WriteBytesExt};
use clap::Arg;
use std::io;
use std::io::Read;

pub enum ReqType {
    Get = 0x0a,
    Scan = 0x0b,
}

struct RocksDBHeader {
    id: u32,
    req_type: u32,
    req_size: u32,
    run_ns: u32,
}

const HEADER_SIZE: usize = 16;

use Connection;
use LoadgenProtocol;
use Transport;

#[derive(Clone, Copy)]
pub struct RocksDBProtocol {
    nvalues: u64,
    pct_scan: u64,
}

impl LoadgenProtocol for RocksDBProtocol {
    fn gen_req(&self, i: usize, p: &Packet, buf: &mut Vec<u8>) -> u64 {
        // Use first 32 bits of randomness to determine if this is a GET or SCAN req
        let low32 = p.randomness & 0xffffffff;
        let key = (p.randomness >> 32) % self.nvalues;
        let mut req_type = ReqType::Get;

        if low32 % 1000 < self.pct_scan {
            req_type = ReqType::Scan;
        }

        let rtype = req_type as u64;
        // println!("req_type: {}", rtype);
        
        RocksDBHeader {
            id: i as u32,
            req_type: rtype as u32,
            req_size: key as u32,
            // req_size: p.work_iterations as u32,
            run_ns: 0,
        }
        .serialize_into(buf)
        .unwrap();

        return rtype;
    }

    fn read_response(&self, mut sock: &Connection, scratch: &mut [u8]) -> io::Result<(usize, u64)> {
        sock.read_exact(&mut scratch[..HEADER_SIZE])?;
        let header = RocksDBHeader::deserialize(&mut &scratch[..])?;
        Ok((header.id as usize, header.req_type as u64))
    }

    fn read_response_w_server_lat(&self, mut sock: &Connection, scratch: &mut [u8]) -> io::Result<(usize, u64, u64)> {
        sock.read_exact(&mut scratch[..HEADER_SIZE])?;
        let header = RocksDBHeader::deserialize(&mut &scratch[..])?;
        Ok((header.id as usize, header.req_type as u64, (header.run_ns/2100) as u64))
    }
}

impl RocksDBProtocol {
    pub fn with_args(matches: &clap::ArgMatches, tport: Transport) -> Self {
        if let Transport::Tcp = tport {
            panic!("tcp is unsupported by the rocksdb protocol");
        }

        RocksDBProtocol {
            nvalues: 5000, // value_t!(matches, "r_nvalues", u64).unwrap(),
            pct_scan: 5, // value_t!(matches, "pctscan", u64).unwrap(),
	}
    }

    pub fn args<'a, 'b>() -> Vec<clap::Arg<'a, 'b>> {
        vec![
            Arg::with_name("r_nvalues")
        .long("r_nvalues")
        .takes_value(true)
        .default_value("5000")
        .help("RocksDB: number of key value pairs"),
            Arg::with_name("pctscan")
		.long("pctscan")
		.takes_value(true)
		.default_value("0")
		.help("RocksDB: scan requests per 1000 requests"),
	]
    }
}

impl RocksDBHeader {
    pub fn serialize_into<W: io::Write>(&self, writer: &mut W) -> io::Result<()> {
        writer.write_u32::<LittleEndian>(self.id)?;
        writer.write_u32::<LittleEndian>(self.req_type)?;
        writer.write_u32::<LittleEndian>(self.req_size)?;
        writer.write_u32::<LittleEndian>(self.run_ns)?;
        Ok(())
    }

    pub fn deserialize<R: io::Read>(reader: &mut R) -> io::Result<RocksDBHeader> {
        let p = RocksDBHeader {
            id: reader.read_u32::<LittleEndian>()?,
            req_type: reader.read_u32::<LittleEndian>()?,
            req_size: reader.read_u32::<LittleEndian>()?,
            run_ns: reader.read_u32::<LittleEndian>()?,
        };
        return Ok(p);
    }
}
